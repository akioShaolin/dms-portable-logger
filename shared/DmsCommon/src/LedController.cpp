#include "LedController.h"
#ifdef ARDUINO
#include <Arduino.h>
#include "Ed100Pins.h"
namespace dms {static void draw(int r,int g,LedPattern p,uint32_t n){bool red=false,green=false;switch(p){case LedPattern::GREEN:green=true;break;case LedPattern::AMBER:red=green=true;break;case LedPattern::RED:red=true;break;case LedPattern::RED_SLOW:red=(n%2000)<1000;break;case LedPattern::AMBER_FAST:red=green=(n%300)<150;break;case LedPattern::PULSE_GREEN:green=(n%1000)<80;break;case LedPattern::PULSE_RED:red=(n%1000)<80;break;case LedPattern::RED_DOUBLE:red=(n%1600)<120||((n%1600)>240&&(n%1600)<360);break;default:break;}digitalWrite(r,red);digitalWrite(g,green);}void LedController::begin(){for(int p:{pins::WIFI_RED,pins::WIFI_GREEN,pins::SERVER_RED,pins::SERVER_GREEN,pins::RS485_RED,pins::RS485_GREEN}){pinMode(p,OUTPUT);digitalWrite(p,LOW);}}void LedController::tick(uint32_t n){draw(pins::WIFI_RED,pins::WIFI_GREEN,wifi_,n);draw(pins::SERVER_RED,pins::SERVER_GREEN,server_,n);draw(pins::RS485_RED,pins::RS485_GREEN,rs485_,n);}}
#else
namespace dms {void LedController::begin(){}void LedController::tick(uint32_t){}}
#endif
