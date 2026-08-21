#include <Arduino.h>
#include <BuildInfo.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <esp_partition.h>
#include "AppConfig.h"
#include <BinaryLogger.h>
#include <BmsSnapshot.h>
#include <DmsLink.h>
#include <JbdCodec.h>
#include <LedController.h>
#include <LogFormat.h>
#include <Rs485Port.h>
#include <RtcClock.h>
#include <StorageConfig.h>
#include <WebPortal.h>
using namespace dms;

struct LogItem { LogType type=LogType::SYSTEM_EVENT; uint16_t length=0; uint8_t data[320]{}; };
static Rs485Port panel;static SnapshotStore snapshots;static RtcClock rtc;static LedController leds;static BinaryLogger logger;static WebPortal web;
static QueueHandle_t logQueue;static uint16_t transactionId;static uint32_t leaseSeq,leaseUntil,lastBackground,lastTimeSync,bootId,droppedLogs,linkFailures,panelRequests,proxyRequests,cacheResponses,writeBlocked;
static JbdFrame rawCache[3];static uint32_t rawCacheMs[3],rawCacheSeq[3];
static char deviceId[13]{};

static void enqueue(LogItem&i){if(xQueueSend(logQueue,&i,0)!=pdTRUE)droppedLogs++;}
static bool rpc(LinkFrame&q,LinkFrame&a,uint32_t timeout=config::PANEL_TIMEOUT_MS){
  q.transactionId=++transactionId;uint8_t wire[DMSLINK_MAX_WIRE];size_t n=encodeLink(q,wire,sizeof wire);if(!n)return false;Serial.write(wire,n);
  size_t used=0;uint32_t until=millis()+timeout;
  while(int32_t(millis()-until)<0){while(Serial.available()){uint8_t b=Serial.read();if(used<sizeof wire)wire[used++]=b;else used=0;if(b==0){if(decodeLink(wire,used,a)&&a.transactionId==q.transactionId&&a.type==(q.type|0x80))return true;used=0;}}vTaskDelay(1);}return false;
}
static bool fetchRaw(uint8_t cmd,uint32_t preferred,JbdFrame&out,uint32_t&sourceSeq,uint32_t&age,uint8_t&flags){
  uint8_t index=cmd-3;LinkFrame q,a;q.type=GET_JBD_RESPONSE;q.length=9;uint8_t*p=q.payload;*p++=cmd;put32(p,preferred);put32(p,config::STALE_FALLBACK_MS);
  if(rpc(q,a)&&a.status==LinkStatus::OK&&a.length>=12){const uint8_t*r=a.payload;sourceSeq=get32(r);age=get32(r);flags=a.flags|uint8_t(get16(r));uint16_t n=get16(r);if(n+12==a.length&&validateJbdResponse(r,n,cmd,&out)==JbdResult::OK){rawCache[index]=out;rawCacheMs[index]=millis()-age;rawCacheSeq[index]=sourceSeq;return true;}}
  age=millis()-rawCacheMs[index];if(rawCache[index].size&&age<=config::STALE_FALLBACK_MS){out=rawCache[index];sourceSeq=rawCacheSeq[index];flags=STALE_RESPONSE|CACHE_USED;leds.setRs485(LedPattern::AMBER_FAST);return true;}return false;
}
static bool proxy(const JbdFrame&req,JbdFrame&out,uint32_t&latency){LinkFrame q,a;q.type=PROXY_JBD_READ;q.length=req.size+2;uint8_t*p=q.payload;put16(p,req.size);memcpy(p,req.bytes,req.size);if(!rpc(q,a,650)||a.status!=LinkStatus::OK||a.length<6)return false;const uint8_t*r=a.payload;latency=get32(r);uint16_t n=get16(r);return n+6==a.length&&validateJbdResponse(r,n,req.command,&out)==JbdResult::OK;}
static void logTransaction(const JbdFrame&req,const JbdFrame&resp,uint32_t latency,uint32_t seq,uint32_t age,uint8_t flags){LogItem i;i.type=LogType::PANEL_TRANSACTION;uint8_t*p=i.data;put32(p,millis());put32(p,latency);put32(p,seq);put32(p,age);*p++=req.command;*p++=flags;put16(p,req.size);memcpy(p,req.bytes,req.size);p+=req.size;put16(p,resp.size);memcpy(p,resp.bytes,resp.size);p+=resp.size;i.length=p-i.data;enqueue(i);}
static void logRawTransaction(const JbdFrame&req,const JbdFrame&resp){LogItem i;i.type=LogType::RAW_JBD_FRAME;uint8_t*p=i.data;put32(p,millis());*p++=req.command;put16(p,req.size);memcpy(p,req.bytes,req.size);p+=req.size;put16(p,resp.size);memcpy(p,resp.bytes,resp.size);p+=resp.size;i.length=p-i.data;enqueue(i);}
static void background(){
  LinkFrame q,a;q.type=GET_SNAPSHOT;bool linked=false;
  if(rpc(q,a,300)&&a.status==LinkStatus::OK){BmsSnapshot s;uint32_t age;if(deserializeSample(a.payload,a.length,s,age)){linked=true;s.captureMonotonicMs=millis()-age;s.quality|=LINK_ONLINE;s.droppedLogs=droppedLogs;snapshots.publish(s);if(s.basic.protection)leds.setRs485(LedPattern::RED_DOUBLE);LogItem i;i.type=LogType::SLAVE_SAMPLE;i.length=serializeSample(s,age,i.data,sizeof i.data);enqueue(i);}}
  if(linked)linkFailures=0;else if(++linkFailures>=3)leds.setRs485(LedPattern::RED_SLOW);
  if(millis()-lastTimeSync>=60000){lastTimeSync=millis();LinkFrame tq,ta;tq.type=GET_TIME;if(rpc(tq,ta)&&ta.status==LinkStatus::OK&&ta.length==13){const uint8_t*p=ta.payload;uint64_t epoch=get64(p);uint8_t quality=*p++;uint64_t old=0;rtc.read(old);if(epoch&&quality&&rtc.write(epoch,ClockQuality::LINK_SYNC)){LogItem i;i.type=LogType::TIME_EVENT;uint8_t*w=i.data;put64(w,old);put64(w,epoch);*w++=uint8_t(ClockQuality::LINK_SYNC);i.length=w-i.data;enqueue(i);}}}
}
static void panelTask(void*){
  esp_task_wdt_add(nullptr);uint8_t buffer[JBD_MAX_FRAME];size_t used=0;
  for(;;){esp_task_wdt_reset();bool activity=false;
    while(Serial2.available()){activity=true;uint8_t b=Serial2.read();if(!used&&b!=0xDD)continue;if(used<sizeof buffer)buffer[used++]=b;else used=0;
      if(used>=4&&used==size_t(buffer[3])+7){JbdFrame req,response;JbdResult result=validateJbdRequest(buffer,used,&req);uint32_t began=millis(),seq=0,age=0;uint8_t flags=0;bool ok=false;
        if(result==JbdResult::OK){
          if(req.bytes[1]==0x5A){response.size=encodeJbdError(req.command,0x80,response.bytes,sizeof response.bytes);response.command=req.command;ok=true;flags=0x80;}
          else if(req.command==3||req.command==4||req.command==5){uint32_t preferred=int32_t(millis()-leaseUntil)<0?leaseSeq:0;ok=fetchRaw(req.command,preferred,response,seq,age,flags);if(ok&&!preferred){leaseSeq=seq;leaseUntil=millis()+config::BURST_LEASE_MS;}}
          else ok=proxy(req,response,age);
          if(!ok){response=JbdFrame{};response.size=encodeJbdError(req.command,0x80,response.bytes,sizeof response.bytes);response.command=req.command;}
          uint32_t latency=millis()-began;panel.transmit(response.bytes,response.size,pdMS_TO_TICKS(20+response.size*12));logTransaction(req,response,latency,seq,age,flags);if(req.bytes[1]==0x5A||req.command<3||req.command>5||!ok)logRawTransaction(req,response);panelRequests++;if(req.bytes[1]==0x5A)writeBlocked++;else if(req.command!=3&&req.command!=4&&req.command!=5)proxyRequests++;if(flags&CACHE_USED)cacheResponses++;BmsSnapshot live;if(snapshots.latest(live)){live.panelRequests=panelRequests;live.proxyRequests=proxyRequests;live.cacheResponses=cacheResponses;live.writeBlocked=writeBlocked;live.lastPanelLatencyMs=latency;live.sourceAgeMs=age;live.lastPanelCommand=req.command;live.lastPanelFlags=flags;snapshots.publish(live);}leds.setRs485(ok?LedPattern::PULSE_GREEN:LedPattern::PULSE_RED);
        }else{LogItem i;i.type=LogType::RAW_JBD_FRAME;i.length=used<sizeof i.data?used:sizeof i.data;memcpy(i.data,buffer,i.length);enqueue(i);leds.setRs485(LedPattern::PULSE_RED);}used=0;
      }
    }
    if(!activity&&millis()-lastBackground>=1000){lastBackground=millis();background();}vTaskDelay(1);
  }
}
static void loggerTask(void*){LogItem i;uint32_t flush=millis();for(;;){if(xQueueReceive(logQueue,&i,pdMS_TO_TICKS(100))==pdTRUE){logger.append(i.type,i.data,i.length);if(i.type==LogType::TIME_EVENT&&i.length>=16){const uint8_t*p=i.data;uint64_t old=get64(p),now=get64(p);if(old&&now<old)logger.rotate();}}if(millis()-flush>=5000){logger.flush();flush=millis();}}}
static void webTask(void*){bool ok=web.begin("Slave",config::WIFI_CHANNEL,config::AP_PASSWORD,snapshots,logger,rtc);leds.setWifi(ok?LedPattern::GREEN:LedPattern::RED);for(;;){web.handle();vTaskDelay(2);}}
void setup(){bootId=esp_random();snprintf(deviceId,sizeof deviceId,"%012llX",ESP.getEfuseMac());Serial.begin(config::LINK_BAUD,SERIAL_8N1,3,1);panel.begin(config::JBD_BAUD);leds.begin();rtc.begin();uint64_t startEpoch=0;rtc.read(startEpoch);bool partition=esp_partition_find_first(ESP_PARTITION_TYPE_DATA,ESP_PARTITION_SUBTYPE_DATA_SPIFFS,LITTLEFS_PARTITION_LABEL)!=nullptr;bool fs=partition&&LittleFS.begin(false,LITTLEFS_BASE_PATH,LITTLEFS_MAX_OPEN_FILES,LITTLEFS_PARTITION_LABEL);logger.setStorageState(partition,fs,partition?StorageError::MOUNT_FAILED:StorageError::PARTITION_NOT_FOUND);bool logOk=fs&&logger.begin(LittleFS,1,bootId,esp_random(),deviceId,DMS_FIRMWARE_VERSION,DMS_GIT_SHA,uint8_t(rtc.quality()),startEpoch);if(logOk){uint8_t event[9],*p=event;put32(p,ESP.getFlashChipSize());put32(p,4194304);*p++=uint8_t(esp_reset_reason());logOk=logger.append(LogType::SYSTEM_EVENT,event,p-event)&&logger.flush();}leds.setServer(logOk?LedPattern::GREEN:LedPattern::RED);logQueue=xQueueCreate(12,sizeof(LogItem));xTaskCreatePinnedToCore(panelTask,"panel",7168,nullptr,5,nullptr,1);xTaskCreatePinnedToCore(loggerTask,"logger",4096,nullptr,1,nullptr,0);xTaskCreatePinnedToCore(webTask,"web",6144,nullptr,2,nullptr,0);}
void loop(){leds.setWifi(WiFi.softAPgetStationNum()?LedPattern::AMBER:LedPattern::GREEN);const auto&s=logger.stats();if(!s.mounted||!s.running)leds.setServer(LedPattern::RED);else if(s.total&&s.used*100/s.total>=95)leds.setServer(LedPattern::RED_SLOW);else if(s.total&&s.used*100/s.total>=85)leds.setServer(LedPattern::AMBER);else leds.setServer(LedPattern::GREEN);leds.tick(millis());vTaskDelay(10);}
