#pragma once
#include <stddef.h>
#include <stdint.h>
namespace dms {constexpr uint16_t LOG_SYNC=0xD25A;enum class LogType:uint8_t{MASTER_SAMPLE=1,POLL_ERROR,SLAVE_SAMPLE,PANEL_TRANSACTION,RAW_JBD_FRAME,LINK_EVENT,SYSTEM_EVENT,TIME_EVENT};uint32_t crc32(const uint8_t*,size_t);size_t serializeSample(const struct BmsSnapshot&,uint32_t age,uint8_t*,size_t);bool deserializeSample(const uint8_t*,size_t,struct BmsSnapshot&,uint32_t&age);}
