#pragma once
#include <stdint.h>
#ifdef ARDUINO
#include <Wire.h>
#include <freertos/semphr.h>
#endif
namespace dms { enum class ClockQuality:uint8_t{UNSET,RTC_VALID,BROWSER_SET,LINK_SYNC};struct DateTime{uint16_t year;uint8_t month,day,hour,minute,second;};bool validDate(const DateTime&);uint64_t dateToEpochMs(const DateTime&);bool epochMsToDate(uint64_t,DateTime&);
#ifdef ARDUINO
class RtcClock{public:bool begin();bool read(uint64_t&);bool write(uint64_t,ClockQuality);ClockQuality quality()const{return quality_;}private:ClockQuality quality_=ClockQuality::UNSET;SemaphoreHandle_t mutex_=nullptr;};
#endif
}
