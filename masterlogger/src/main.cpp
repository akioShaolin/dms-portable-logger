#include <Arduino.h>
#include <BuildInfo.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include "AppConfig.h"
#include <BinaryLogger.h>
#include <BmsSnapshot.h>
#include <DmsLink.h>
#include <JbdCodec.h>
#include <LedController.h>
#include <LogFormat.h>
#include <RegisterBank.h>
#include <Rs485Port.h>
#include <RtcClock.h>
#include <WebPortal.h>
using namespace dms;

struct ProxyJob { JbdFrame request{},response{}; TaskHandle_t waiter=nullptr; LinkStatus result=LinkStatus::BUSY; uint32_t latency=0; };
struct LogMessage { LogType type=LogType::SYSTEM_EVENT; BmsSnapshot snapshot{}; uint16_t length=0; uint8_t data[320]{}; };

static Rs485Port bus; static SnapshotStore snapshots; static RegisterBank registers;
static RtcClock rtc; static LedController leds; static BinaryLogger logger; static WebPortal web;
static QueueHandle_t proxyQueue,logQueue; static uint32_t bootId,sequence,droppedLogs,checksumErrors,invalidFrames;
static JbdFrame hardwareFrame; static char hardware[32]{},deviceId[13]{}; static uint32_t hardwareMs;

static void enqueue(LogMessage&m){if(xQueueSend(logQueue,&m,0)!=pdTRUE)droppedLogs++;}
static bool transact(const uint8_t*q,size_t n,uint8_t cmd,JbdFrame&frame,uint32_t&latency){
  uint32_t start=millis(); if(!bus.transmit(q,n,pdMS_TO_TICKS(20+n*12)))return false;
  JbdStreamParser parser; JbdResult error=JbdResult::INCOMPLETE; uint32_t until=millis()+config::RESPONSE_TIMEOUT_MS;
  while(int32_t(millis()-until)<0){while(Serial2.available()){if(parser.push(uint8_t(Serial2.read()),cmd,frame,error)){latency=millis()-start;return true;}if(error!=JbdResult::INCOMPLETE){if(error==JbdResult::BAD_CHECKSUM)checksumErrors++;invalidFrames++;return false;}}vTaskDelay(1);}
  return false;
}
static bool command(uint8_t cmd,JbdFrame&f,uint32_t&latency){uint8_t q[7];size_t n=encodeJbdRead(cmd,q,sizeof q);return transact(q,n,cmd,f,latency);}
static void logPollError(uint8_t stage){LogMessage m;m.type=LogType::POLL_ERROR;uint8_t*p=m.data;put32(p,millis());*p++=stage;put32(p,sequence);m.length=p-m.data;enqueue(m);}
static void logProxy(const ProxyJob&job){LogMessage m;m.type=LogType::RAW_JBD_FRAME;uint8_t*p=m.data;put32(p,millis());*p++=job.request.command;put16(p,job.request.size);memcpy(p,job.request.bytes,job.request.size);p+=job.request.size;put16(p,job.response.size);memcpy(p,job.response.bytes,job.response.size);p+=job.response.size;m.length=p-m.data;enqueue(m);}

static void bmsTask(void*){
  esp_task_wdt_add(nullptr); uint32_t deadline=millis(),nextHardware=0,timeouts=0,offlineFailures=0;
  for(;;){
    esp_task_wdt_reset(); ProxyJob*job=nullptr;
    if(xQueueReceive(proxyQueue,&job,0)==pdTRUE){uint32_t began=millis();job->result=transact(job->request.bytes,job->request.size,job->request.command,job->response,job->latency)?LinkStatus::OK:LinkStatus::BMS_TIMEOUT;job->latency=millis()-began;logProxy(*job);xTaskNotifyGive(job->waiter);continue;}
    if(int32_t(millis()-deadline)<0){vTaskDelay(1);continue;}deadline+=config::POLL_MS;
    BmsSnapshot s;uint32_t began=micros(),latency=0;uint8_t failedStage=3;
    bool ok=command(3,s.raw03,latency);if(ok){failedStage=4;ok=command(4,s.raw04,latency);}if(ok)ok=decodeBasic(s.raw03,s.basic)&&decodeCells(s.raw04,s.cells);
    if(ok){
      s.bootId=bootId;s.sequence=++sequence;s.captureMonotonicMs=millis();s.quality=BASIC_VALID|CELLS_VALID|BMS_ONLINE;s.timeouts=timeouts;s.checksumErrors=checksumErrors;s.discardedFrames=invalidFrames;s.droppedLogs=droppedLogs;
      if(rtc.read(s.timestampUtcMs))s.quality|=RTC_VALID;else s.quality|=CLOCK_UNSET;
      if(s.basic.protection)s.quality|=PROTECTION_ACTIVE;if(s.cells.count!=config::EXPECTED_CELL_COUNT)s.quality|=UNEXPECTED_CELL_COUNT;if(s.basic.extraLength)s.quality|=EXTRA_PRESERVED;
      if(hardwareFrame.size){s.raw05=hardwareFrame;s.hardwareCaptureMs=hardwareMs;strncpy(s.hardware,hardware,sizeof s.hardware);s.quality|=HARDWARE_VALID;}
      s.pollDurationUs=micros()-began;s.polls=sequence;snapshots.publish(s);registers.update(s,0,0);
      LogMessage m;m.type=LogType::MASTER_SAMPLE;m.snapshot=s;enqueue(m);offlineFailures=0;leds.setRs485(s.basic.protection?LedPattern::RED_DOUBLE:LedPattern::PULSE_GREEN);
    }else{timeouts++;offlineFailures++;logPollError(failedStage);leds.setRs485(offlineFailures>=3?LedPattern::RED_SLOW:LedPattern::PULSE_RED);}
    if(!hardwareFrame.size||int32_t(millis()-nextHardware)>=0){JbdFrame h;if(command(5,h,latency)&&decodeHardware(h,hardware,sizeof hardware)){hardwareFrame=h;hardwareMs=millis();nextHardware=millis()+3600000;}else nextHardware=millis()+5000;}
  }
}

static void snapshotResponse(LinkFrame&a){BmsSnapshot s;if(!snapshots.latest(s)){a.status=LinkStatus::NO_SNAPSHOT;return;}a.length=serializeSample(s,millis()-s.captureMonotonicMs,a.payload,sizeof a.payload);if(!a.length)a.status=LinkStatus::INTERNAL_ERROR;}
static void registerResponse(const LinkFrame&q,LinkFrame&a){if(q.length!=4){a.status=LinkStatus::BAD_REQUEST;return;}const uint8_t*p=q.payload;uint16_t address=get16(p),count=get16(p);if(count>180||uint32_t(address)+count>REGISTER_COUNT){a.status=LinkStatus::BAD_REQUEST;return;}uint16_t words[180];if(!registers.read(address,count,words,180)){a.status=LinkStatus::INTERNAL_ERROR;return;}uint8_t*w=a.payload;put16(w,address);put16(w,count);for(uint16_t i=0;i<count;i++)put16(w,words[i]);a.length=w-a.payload;}
static void rawResponse(const LinkFrame&q,LinkFrame&a){
  if(q.length!=9){a.status=LinkStatus::BAD_REQUEST;return;}const uint8_t*p=q.payload;uint8_t cmd=*p++;uint32_t preferred=get32(p),maxAge=get32(p);BmsSnapshot s;
  bool exact=preferred?snapshots.bySequence(preferred,s):snapshots.latest(s);if(!exact&&preferred&&snapshots.latest(s))a.flags|=SEQ_SUBSTITUTED;if(!s.sequence){a.status=LinkStatus::NO_SNAPSHOT;return;}
  uint32_t age=cmd==5?millis()-s.hardwareCaptureMs:millis()-s.captureMonotonicMs;if(age>maxAge){a.status=LinkStatus::STALE;return;}if(age>config::FRESH_AGE_MS)a.flags|=STALE_RESPONSE;
  const JbdFrame*f=cmd==3?&s.raw03:cmd==4?&s.raw04:cmd==5?&s.raw05:nullptr;if(!f||!f->size){a.status=LinkStatus::UNSUPPORTED_COMMAND;return;}
  uint8_t*w=a.payload;put32(w,s.sequence);put32(w,age);put16(w,a.flags);put16(w,f->size);memcpy(w,f->bytes,f->size);w+=f->size;a.length=w-a.payload;
}
static void proxyResponse(const LinkFrame&q,LinkFrame&a){
  if(q.length<2){a.status=LinkStatus::BAD_REQUEST;return;}const uint8_t*p=q.payload;uint16_t n=get16(p);JbdFrame req;
  if(n+2!=q.length||validateJbdRequest(p,n,&req)!=JbdResult::OK){a.status=LinkStatus::BAD_REQUEST;return;}if(req.bytes[1]!=0xA5){a.status=LinkStatus::WRITE_BLOCKED;return;}
  ProxyJob job;job.request=req;job.waiter=xTaskGetCurrentTaskHandle();ProxyJob*ptr=&job;
  if(xQueueSend(proxyQueue,&ptr,pdMS_TO_TICKS(20))!=pdTRUE||ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(600))==0){a.status=LinkStatus::BUSY;return;}a.status=job.result;
  if(a.status==LinkStatus::OK){uint8_t*w=a.payload;put32(w,job.latency);put16(w,job.response.size);memcpy(w,job.response.bytes,job.response.size);w+=job.response.size;a.length=w-a.payload;a.flags|=PROXIED;}
}
static void makeResponse(const LinkFrame&q,LinkFrame&a){
  a=LinkFrame{};a.type=q.type|0x80;a.transactionId=q.transactionId;
  if(q.type==HELLO){uint8_t*p=a.payload;*p++=1;*p++=0;put16(p,SNAPSHOT_SCHEMA);put32(p,bootId);put32(p,4194304);a.length=p-a.payload;}
  else if(q.type==PING){uint8_t*p=a.payload;put32(p,millis());put32(p,uxTaskGetStackHighWaterMark(nullptr));put32(p,droppedLogs);a.length=p-a.payload;}
  else if(q.type==GET_TIME){uint64_t epoch=0;rtc.read(epoch);uint8_t*p=a.payload;put64(p,epoch);*p++=uint8_t(rtc.quality());put32(p,millis());a.length=p-a.payload;}
  else if(q.type==GET_SNAPSHOT)snapshotResponse(a);else if(q.type==READ_REGISTERS)registerResponse(q,a);else if(q.type==GET_JBD_RESPONSE)rawResponse(q,a);else if(q.type==PROXY_JBD_READ)proxyResponse(q,a);else a.status=LinkStatus::UNSUPPORTED_COMMAND;
}
static void linkServerTask(void*){
  esp_task_wdt_add(nullptr);uint8_t wire[DMSLINK_MAX_WIRE];size_t used=0;LinkFrame lastRequest{},lastResponse{};bool cached=false;
  for(;;){esp_task_wdt_reset();while(Serial.available()){uint8_t b=Serial.read();if(used<sizeof wire)wire[used++]=b;else used=0;if(b==0){LinkFrame q,a;if(decodeLink(wire,used,q)){if(cached&&q.transactionId==lastRequest.transactionId&&q.type==lastRequest.type)a=lastResponse;else{makeResponse(q,a);lastRequest=q;lastResponse=a;cached=true;}size_t n=encodeLink(a,wire,sizeof wire);Serial.write(wire,n);}used=0;}}vTaskDelay(1);}
}
static void loggerTask(void*){LogMessage m;uint32_t lastFlush=millis();for(;;){if(xQueueReceive(logQueue,&m,pdMS_TO_TICKS(100))==pdTRUE){if(m.type==LogType::MASTER_SAMPLE){uint8_t p[224];size_t n=serializeSample(m.snapshot,0,p,sizeof p);logger.append(m.type,p,n);}else logger.append(m.type,m.data,m.length);}if(millis()-lastFlush>=5000){logger.flush();lastFlush=millis();}}}
static void webTask(void*){bool ok=web.begin("Master",config::WIFI_CHANNEL,config::AP_PASSWORD,snapshots,logger,rtc);leds.setWifi(ok?LedPattern::GREEN:LedPattern::RED);for(;;){web.handle();vTaskDelay(2);}}
void setup(){
  bootId=esp_random();snprintf(deviceId,sizeof deviceId,"%012llX",ESP.getEfuseMac());Serial.begin(config::LINK_BAUD,SERIAL_8N1,3,1);bus.begin(config::JBD_BAUD);leds.begin();rtc.begin();uint64_t startEpoch=0;rtc.read(startEpoch);bool fs=LittleFS.begin(false);
  if(fs){logger.begin(LittleFS,0,bootId,esp_random(),deviceId,DMS_FIRMWARE_VERSION,DMS_GIT_SHA,uint8_t(rtc.quality()),startEpoch);uint8_t event[9],*p=event;put32(p,ESP.getFlashChipSize());put32(p,4194304);*p++=uint8_t(esp_reset_reason());logger.append(LogType::SYSTEM_EVENT,event,p-event);}leds.setServer(fs?LedPattern::GREEN:LedPattern::RED);proxyQueue=xQueueCreate(4,sizeof(ProxyJob*));logQueue=xQueueCreate(8,sizeof(LogMessage));
  xTaskCreatePinnedToCore(bmsTask,"bms",6144,nullptr,5,nullptr,1);xTaskCreatePinnedToCore(linkServerTask,"link",6144,nullptr,5,nullptr,1);xTaskCreatePinnedToCore(loggerTask,"logger",4096,nullptr,1,nullptr,0);xTaskCreatePinnedToCore(webTask,"web",6144,nullptr,2,nullptr,0);
}
void loop(){leds.setWifi(WiFi.softAPgetStationNum()?LedPattern::AMBER:LedPattern::GREEN);const auto&s=logger.stats();if(!s.mounted||!s.running)leds.setServer(LedPattern::RED);else if(s.total&&s.used*100/s.total>=95)leds.setServer(LedPattern::RED_SLOW);else if(s.total&&s.used*100/s.total>=85)leds.setServer(LedPattern::AMBER);else leds.setServer(LedPattern::GREEN);leds.tick(millis());vTaskDelay(10);}
