#pragma once
#include <stddef.h>
#include <stdint.h>
namespace dms {
constexpr size_t JBD_MAX_FRAME=128;
enum class JbdResult:uint8_t { OK, INCOMPLETE, BAD_START, BAD_ACTION, BAD_LENGTH, BAD_END, BAD_CHECKSUM, BAD_COMMAND, BAD_STATUS };
struct JbdFrame { uint8_t bytes[JBD_MAX_FRAME]{}; uint16_t size=0; uint8_t command=0,status=0,payloadLength=0; };
struct BasicInfo { uint32_t voltage_mV=0; int32_t current_mA=0; uint32_t remaining_mAh=0, nominal_mAh=0; int64_t power_mW=0; uint16_t cycles=0,productionRaw=0,balanceLow=0,balanceHigh=0,protection=0; uint16_t year=0; uint8_t month=0,day=0,softwareMajor=0,softwareMinor=0,rsoc=0,fets=0,cellCount=0,ntcCount=0,extraLength=0; int16_t temperatures_dC[8]{}; uint8_t extra[32]{}; };
struct CellInfo { uint8_t count=0,minIndex=0,maxIndex=0; uint16_t mV[32]{},minimum_mV=0,maximum_mV=0,average_mV=0,delta_mV=0; };
uint16_t jbdChecksum(const uint8_t* data,size_t len);
size_t encodeJbdRead(uint8_t command,uint8_t* out,size_t capacity);
size_t encodeJbdError(uint8_t command,uint8_t status,uint8_t* out,size_t capacity);
JbdResult validateJbdRequest(const uint8_t* data,size_t len,JbdFrame* out=nullptr);
JbdResult validateJbdResponse(const uint8_t* data,size_t len,uint8_t expected,JbdFrame* out=nullptr);
bool decodeBasic(const JbdFrame& frame,BasicInfo& out);
bool decodeCells(const JbdFrame& frame,CellInfo& out);
class JbdStreamParser { public: bool push(uint8_t b,JbdFrame& out,JbdResult& error); void reset(); private: uint8_t b_[JBD_MAX_FRAME]{}; uint16_t n_=0,expected_=0; };
}
