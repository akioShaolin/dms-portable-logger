#include "RtcClock.h"
namespace dms {
static bool leap(unsigned y){return y%4==0&&(y%100!=0||y%400==0);}
static uint8_t mdays(unsigned y,unsigned m){static const uint8_t d[]={31,28,31,30,31,30,31,31,30,31,30,31};return m==2?d[1]+leap(y):m&&m<13?d[m-1]:0;}
bool validDate(const DateTime&d){return d.year>=2000&&d.year<=2099&&d.month&&d.month<=12&&d.day&&d.day<=mdays(d.year,d.month)&&d.hour<24&&d.minute<60&&d.second<60;}
uint64_t dateToEpochMs(const DateTime&d){if(!validDate(d))return 0;uint64_t days=0;for(unsigned y=1970;y<d.year;y++)days+=365+leap(y);for(unsigned m=1;m<d.month;m++)days+=mdays(d.year,m);days+=d.day-1;return (((days*24+d.hour)*60+d.minute)*60+d.second)*1000;}
bool epochMsToDate(uint64_t e,DateTime&d){uint64_t s=e/1000,days=s/86400;s%=86400;d.hour=s/3600;d.minute=(s/60)%60;d.second=s%60;unsigned y=1970;while(days>=365u+leap(y))days-=365+leap(y++);unsigned m=1;while(m<=12&&days>=mdays(y,m))days-=mdays(y,m++);d.year=y;d.month=m;d.day=days+1;return validDate(d);}
#ifdef ARDUINO
class RtcLock{public:explicit RtcLock(SemaphoreHandle_t h):h_(h){if(h_)xSemaphoreTake(h_,portMAX_DELAY);}~RtcLock(){if(h_)xSemaphoreGive(h_);}private:SemaphoreHandle_t h_;};
static uint8_t bcd(uint8_t v){return (v>>4)*10+(v&15);}static uint8_t enc(uint8_t v){return (v/10)<<4|v%10;}
bool RtcClock::begin(){mutex_=xSemaphoreCreateMutex();if(!mutex_)return false;Wire.begin(21,22);uint64_t epoch;return read(epoch);}
bool RtcClock::read(uint64_t&e){RtcLock lock(mutex_);Wire.beginTransmission(0x51);Wire.write(2);if(Wire.endTransmission()!=0||Wire.requestFrom(0x51,7)!=7)return false;uint8_t second=Wire.read(),minute=Wire.read(),hour=Wire.read(),day=Wire.read();Wire.read();uint8_t month=Wire.read(),year=Wire.read();if(second&0x80)return false;DateTime d{uint16_t(2000+bcd(year)),bcd(uint8_t(month&0x1f)),bcd(uint8_t(day&0x3f)),bcd(uint8_t(hour&0x3f)),bcd(uint8_t(minute&0x7f)),bcd(uint8_t(second&0x7f))};if(!validDate(d))return false;e=dateToEpochMs(d);if(quality_==ClockQuality::UNSET)quality_=ClockQuality::RTC_VALID;return true;}
bool RtcClock::write(uint64_t e,ClockQuality q){RtcLock lock(mutex_);DateTime d;if(!epochMsToDate(e,d))return false;Wire.beginTransmission(0x51);Wire.write(2);Wire.write(enc(d.second));Wire.write(enc(d.minute));Wire.write(enc(d.hour));Wire.write(enc(d.day));Wire.write(0);Wire.write(enc(d.month));Wire.write(enc(d.year-2000));if(Wire.endTransmission()!=0)return false;quality_=q;return true;}
#endif
}
