#pragma once
#include "LogFormat.h"
#include <stddef.h>
#include <stdint.h>
#ifdef ARDUINO
#include <FS.h>
#include <freertos/semphr.h>
#endif
namespace dms {constexpr size_t LOG_HEADER_SIZE=64,LOG_ROTATE_BYTES=512*1024,LOG_RESERVE_BYTES=64*1024;size_t encodeLogRecord(LogType,uint8_t,const uint8_t*,uint32_t,uint8_t*,size_t);bool logNeedsRotation(uint64_t currentBytes,uint32_t nextRecordBytes);bool logHasReserve(uint64_t totalBytes,uint64_t usedBytes);struct LoggerStats{bool mounted=false,running=false;uint32_t records=0,dropped=0;uint64_t bytes=0,total=0,used=0;uint32_t bytesPerSecond=0;uint32_t estimatedMinutesRemaining=0;};
#ifdef ARDUINO
class BinaryLogger{public:bool begin(fs::FS&,uint8_t role,uint32_t boot,uint32_t session,const char*device,const char*firmware="0.1.0",const char*gitSha="unknown",uint8_t clockQuality=0,uint64_t startEpochMs=0);bool append(LogType,const uint8_t*,uint32_t);bool rotate();void flush();const LoggerStats&stats()const{return stats_;}const char*name()const{return name_;}private:fs::FS*fs_=nullptr;File file_;LoggerStats stats_{};SemaphoreHandle_t mutex_=nullptr;uint8_t role_=0,clockQuality_=0;uint32_t boot_=0,session_=0,index_=0;uint32_t startedMs_=0;uint64_t startEpochMs_=0;char device_[13]{},firmware_[9]{},gitSha_[13]{},name_[64]{};bool open();};
#endif
}
