#include <Arduino.h>
#include <LittleFS.h>
#include "AppConfig.h"
#include <DmsLink.h>
#include <JbdCodec.h>
#include <Rs485Port.h>
using namespace dms;static Rs485Port panel;static uint8_t requestBuf[JBD_MAX_FRAME];static size_t requestLen;
static bool readLinkResponse(uint8_t command,JbdFrame&response){LinkFrame q;q.type=4;q.transactionId=uint16_t(millis());q.length=5;q.payload[0]=command;memset(q.payload+1,0,4);uint8_t wire[DMSLINK_MAX_WIRE];size_t n=encodeLink(q,wire,sizeof wire);Serial.write(wire,n);uint8_t rx[DMSLINK_MAX_WIRE];size_t used=0;uint32_t until=millis()+config::PANEL_TIMEOUT_MS;while(int32_t(millis()-until)<0){while(Serial.available()){uint8_t b=Serial.read();if(used<sizeof rx)rx[used++]=b;if(b==0){LinkFrame a;if(decodeLink(rx,used,a)&&a.status==LinkStatus::OK&&validateJbdResponse(a.payload,a.length,command,&response)==JbdResult::OK)return true;used=0;}}vTaskDelay(1);}return false;}
void setup(){Serial.begin(config::LINK_BAUD,SERIAL_8N1,3,1);panel.begin(config::JBD_BAUD);LittleFS.begin(false);}
void loop(){while(Serial2.available()){uint8_t b=Serial2.read();if(!requestLen&&b!=0xDD)continue;if(requestLen<sizeof requestBuf)requestBuf[requestLen++]=b;else requestLen=0;if(requestLen>=4&&requestLen==size_t(requestBuf[3])+7){JbdFrame req;JbdResult r=validateJbdRequest(requestBuf,requestLen,&req);if(r==JbdResult::OK){uint8_t out[JBD_MAX_FRAME];size_t n=0;if(requestBuf[1]==0x5A)n=encodeJbdError(req.command,0x80,out,sizeof out);else {JbdFrame response;if(readLinkResponse(req.command,response)){memcpy(out,response.bytes,response.size);n=response.size;}else n=encodeJbdError(req.command,0x80,out,sizeof out);}panel.transmit(out,n);}requestLen=0;}}vTaskDelay(1);}
