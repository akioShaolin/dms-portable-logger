#include "LogFormat.h"
#include "BmsSnapshot.h"
#include "DmsLink.h"
#include <string.h>
namespace dms {
uint32_t crc32(const uint8_t*d,size_t n){uint32_t c=~0u;while(n--){c^=*d++;for(int i=0;i<8;i++)c=c&1?(c>>1)^0xEDB88320:c>>1;}return~c;}
size_t serializeSample(const BmsSnapshot&s,uint32_t age,uint8_t*out,size_t cap){
  size_t need=128+2*s.cells.count+2*s.basic.ntcCount;if(cap<need||s.cells.count>32||s.basic.ntcCount>8)return 0;uint8_t*p=out;
  put32(p,s.sequence);put64(p,s.timestampUtcMs);put32(p,s.captureMonotonicMs);put32(p,age);put32(p,s.quality);put32(p,s.basic.voltage_mV);put32(p,uint32_t(s.basic.current_mA));put64(p,uint64_t(s.basic.power_mW));put32(p,s.basic.remaining_mAh);put32(p,s.basic.nominal_mAh);
  *p++=s.basic.rsoc;*p++=s.basic.fets;*p++=s.cells.count;*p++=s.basic.ntcCount;put16(p,s.basic.cycles);put16(p,s.basic.productionRaw);put16(p,s.basic.protection);put16(p,s.basic.balanceLow);put16(p,s.basic.balanceHigh);put16(p,s.cells.minimum_mV);put16(p,s.cells.maximum_mV);put16(p,s.cells.average_mV);put16(p,s.cells.delta_mV);*p++=s.cells.minIndex;*p++=s.cells.maxIndex;
  put32(p,s.pollDurationUs);put32(p,s.timeouts);put32(p,s.checksumErrors);put32(p,s.discardedFrames);put32(p,s.linkErrors);put32(p,s.droppedLogs);memcpy(p,s.hardware,32);p+=32;
  for(uint8_t i=0;i<s.basic.ntcCount;i++)put16(p,uint16_t(s.basic.temperatures_dC[i]));for(uint8_t i=0;i<s.cells.count;i++)put16(p,s.cells.mV[i]);return p-out;
}
bool deserializeSample(const uint8_t*data,size_t n,BmsSnapshot&s,uint32_t&age){
  if(n<128)return false;const uint8_t*p=data;s=BmsSnapshot{};s.sequence=get32(p);s.timestampUtcMs=get64(p);s.captureMonotonicMs=get32(p);age=get32(p);s.quality=get32(p);s.basic.voltage_mV=get32(p);s.basic.current_mA=int32_t(get32(p));s.basic.power_mW=int64_t(get64(p));s.basic.remaining_mAh=get32(p);s.basic.nominal_mAh=get32(p);s.basic.rsoc=*p++;s.basic.fets=*p++;s.cells.count=*p++;s.basic.ntcCount=*p++;
  s.basic.cycles=get16(p);s.basic.productionRaw=get16(p);s.basic.protection=get16(p);s.basic.balanceLow=get16(p);s.basic.balanceHigh=get16(p);s.cells.minimum_mV=get16(p);s.cells.maximum_mV=get16(p);s.cells.average_mV=get16(p);s.cells.delta_mV=get16(p);s.cells.minIndex=*p++;s.cells.maxIndex=*p++;s.pollDurationUs=get32(p);s.timeouts=get32(p);s.checksumErrors=get32(p);s.discardedFrames=get32(p);s.linkErrors=get32(p);s.droppedLogs=get32(p);memcpy(s.hardware,p,32);s.hardware[31]=0;p+=32;
  if(s.cells.count>32||s.basic.ntcCount>8||size_t(p-data)+2*s.cells.count+2*s.basic.ntcCount!=n)return false;for(uint8_t i=0;i<s.basic.ntcCount;i++)s.basic.temperatures_dC[i]=int16_t(get16(p));s.basic.cellCount=s.cells.count;for(uint8_t i=0;i<s.cells.count;i++)s.cells.mV[i]=get16(p);return true;
}
}
