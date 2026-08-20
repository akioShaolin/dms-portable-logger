#include <Arduino.h>
#include <LittleFS.h>
#include "AppConfig.h"
#include <BmsSnapshot.h>
#include <JbdCodec.h>
#include <Rs485Port.h>
using namespace dms;
static Rs485Port bus; static SnapshotStore snapshots; static uint32_t deadline,sequence; static uint8_t request[7];
static bool transact(uint8_t cmd,JbdFrame&frame){size_t n=encodeJbdRead(cmd,request,sizeof request);if(!bus.transmit(request,n))return false;JbdStreamParser p;JbdResult e=JbdResult::INCOMPLETE;uint32_t until=millis()+config::RESPONSE_TIMEOUT_MS;while(int32_t(millis()-until)<0){while(Serial2.available())if(p.push(Serial2.read(),frame,e))return true;vTaskDelay(1);}return false;}
void setup(){Serial.begin(config::LINK_BAUD,SERIAL_8N1,3,1);bus.begin(config::JBD_BAUD);LittleFS.begin(false);deadline=millis();}
void loop(){if(int32_t(millis()-deadline)<0){vTaskDelay(1);return;}deadline+=config::POLL_MS;BmsSnapshot s;uint32_t started=micros();if(transact(3,s.raw03)&&transact(4,s.raw04)&&decodeBasic(s.raw03,s.basic)&&decodeCells(s.raw04,s.cells)){s.sequence=++sequence;s.captureMonotonicMs=millis();s.quality=BASIC_VALID|CELLS_VALID|BMS_ONLINE;if(s.basic.protection)s.quality|=PROTECTION_ACTIVE;if(s.cells.count!=config::EXPECTED_CELL_COUNT)s.quality|=UNEXPECTED_CELL_COUNT;s.pollDurationUs=micros()-started;snapshots.publish(s);}}
