#pragma once
#include <stdint.h>
namespace dms {enum class LedPattern:uint8_t{OFF,GREEN,AMBER,RED,RED_SLOW,AMBER_FAST,PULSE_GREEN,PULSE_RED,RED_DOUBLE};class LedController{public:void begin();void setWifi(LedPattern p){wifi_=p;}void setServer(LedPattern p){server_=p;}void setRs485(LedPattern p){rs485_=p;}void tick(uint32_t now);private:LedPattern wifi_=LedPattern::OFF,server_=LedPattern::OFF,rs485_=LedPattern::OFF;};}
