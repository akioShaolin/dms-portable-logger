#pragma once
#include "LogFormat.h"
#include "StorageConfig.h"
#include <stddef.h>
#include <stdint.h>
#ifdef ARDUINO
#include <FS.h>
#include <freertos/semphr.h>
#endif
namespace dms {
constexpr size_t LOG_HEADER_SIZE=64,LOG_ROTATE_BYTES=512*1024,LOG_RESERVE_BYTES=64*1024;
size_t encodeLogRecord(LogType,uint8_t,const uint8_t*,uint32_t,uint8_t*,size_t);
bool logNeedsRotation(uint64_t,uint32_t);bool logHasReserve(uint64_t,uint64_t);bool validLogHeader(const uint8_t*,size_t);
struct LoggerStats{bool partitionFound=false,mounted=false,running=false;uint64_t total=0,used=0,bytes=0;uint32_t records=0,dropped=0,bytesPerSecond=0,estimatedMinutesRemaining=0,rotationCount=0,lastWriteMs=0,lastFlushMs=0;StorageError error=StorageError::MOUNT_FAILED;};
#ifdef ARDUINO
class BinaryLogger{public:void setStorageState(bool,bool,StorageError);bool begin(fs::FS&,uint8_t,uint32_t,uint32_t,const char*,const char* = "0.1.0",const char* = "unknown",uint8_t=0,uint64_t=0);bool append(LogType,const uint8_t*,uint32_t);bool rotate();bool flush();const LoggerStats&stats()const{return stats_;}const char*name()const{return name_;}private:fs::FS*fs_=nullptr;File file_;LoggerStats stats_{};SemaphoreHandle_t mutex_=nullptr;uint8_t role_=0,clockQuality_=0;uint32_t boot_=0,session_=0,index_=0,startedMs_=0;uint64_t startEpochMs_=0;char device_[13]{},firmware_[9]{},gitSha_[13]{},name_[64]{};bool open();void fail(StorageError);};
#endif
}
