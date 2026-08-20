#pragma once
#ifdef ARDUINO
#include <Arduino.h>
#include <driver/uart.h>
namespace dms {struct Rs485Stats{uint32_t txFrames=0,txTimeouts=0,discardedDuringTx=0;};class Rs485Port { public: explicit Rs485Port(HardwareSerial&s=Serial2):serial_(s){} void begin(uint32_t baud=9600); bool transmit(const uint8_t*,size_t,TickType_t timeout=pdMS_TO_TICKS(250)); HardwareSerial& serial(){return serial_;}const Rs485Stats&stats()const{return stats_;} private:HardwareSerial&serial_;Rs485Stats stats_{};}; }
#endif
