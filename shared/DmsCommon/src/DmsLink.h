#pragma once
#include <stddef.h>
#include <stdint.h>
namespace dms { constexpr size_t DMSLINK_MAX_PAYLOAD=384,DMSLINK_MAX_RAW=396,DMSLINK_MAX_WIRE=512; enum class LinkStatus:uint8_t{OK,BAD_REQUEST,BAD_CRC,UNSUPPORTED_VERSION,UNSUPPORTED_COMMAND,NO_SNAPSHOT,STALE,SEQ_NOT_FOUND,BMS_TIMEOUT,BMS_INVALID_FRAME,WRITE_BLOCKED,BUSY,INTERNAL_ERROR}; struct LinkFrame{uint8_t type=0,flags=0;LinkStatus status=LinkStatus::OK;uint16_t transactionId=0,length=0;uint8_t payload[DMSLINK_MAX_PAYLOAD]{};};uint16_t crc16Ccitt(const uint8_t*,size_t);size_t cobsEncode(const uint8_t*,size_t,uint8_t*,size_t);size_t cobsDecode(const uint8_t*,size_t,uint8_t*,size_t);size_t encodeLink(const LinkFrame&,uint8_t*,size_t);bool decodeLink(const uint8_t*,size_t,LinkFrame&); }
