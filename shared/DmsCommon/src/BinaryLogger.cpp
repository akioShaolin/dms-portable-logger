#include "BinaryLogger.h"
#include "DmsLink.h"
#include <string.h>
#ifdef ARDUINO
class LoggerLock{public:explicit LoggerLock(SemaphoreHandle_t h):h_(h){if(h_)xSemaphoreTakeRecursive(h_,portMAX_DELAY);}~LoggerLock(){if(h_)xSemaphoreGiveRecursive(h_);}private:SemaphoreHandle_t h_;};
#include <LittleFS.h>
#include <stdio.h>
#endif
namespace dms {
size_t encodeLogRecord(LogType type,uint8_t version,const uint8_t*payload,uint32_t length,uint8_t*out,size_t capacity){if(length>65536||capacity<size_t(length)+12)return 0;uint8_t*p=out;put16(p,LOG_SYNC);*p++=uint8_t(type);*p++=version;put32(p,length);if(length)memcpy(p,payload,length);p+=length;put32(p,crc32(out,8+length));return p-out;}
bool logNeedsRotation(uint64_t currentBytes,uint32_t nextRecordBytes){return currentBytes&&currentBytes+nextRecordBytes>LOG_ROTATE_BYTES;}
bool logHasReserve(uint64_t totalBytes,uint64_t usedBytes){return totalBytes>usedBytes&&totalBytes-usedBytes>=LOG_RESERVE_BYTES;}
#ifdef ARDUINO
bool BinaryLogger::begin(fs::FS&fs,uint8_t role,uint32_t boot,uint32_t session,const char*device,const char*firmware,const char*gitSha,uint8_t clockQuality,uint64_t startEpochMs){mutex_=xSemaphoreCreateRecursiveMutex();if(!mutex_)return false;fs_=&fs;role_=role;boot_=boot;session_=session;clockQuality_=clockQuality;startEpochMs_=startEpochMs;startedMs_=millis();strncpy(device_,device,sizeof(device_)-1);strncpy(firmware_,firmware,sizeof(firmware_)-1);strncpy(gitSha_,gitSha,sizeof(gitSha_)-1);stats_.mounted=true;stats_.total=LittleFS.totalBytes();stats_.used=LittleFS.usedBytes();return open();}
bool BinaryLogger::open(){snprintf(name_,sizeof name_,"/%c-%08lx-%08lx-%03lu.dmslog",role_?'S':'M',(unsigned long)boot_,(unsigned long)session_,(unsigned long)index_++);file_=fs_->open(name_,FILE_WRITE);if(!file_)return stats_.running=false;uint8_t h[LOG_HEADER_SIZE]{};memcpy(h,"DMSLOG2",7);h[7]=2;h[8]=role_;h[9]=clockQuality_;uint8_t*p=h+10;put16(p,1);put32(p,boot_);put32(p,session_);put64(p,startEpochMs_);memcpy(h+28,device_,12);memcpy(h+40,firmware_,8);memcpy(h+48,gitSha_,12);uint32_t c=crc32(h,60);p=h+60;put32(p,c);if(file_.write(h,sizeof h)!=sizeof h)return stats_.running=false;stats_.bytes+=sizeof h;return stats_.running=true;}
bool BinaryLogger::append(LogType type,const uint8_t*payload,uint32_t length){LoggerLock lock(mutex_);if(!stats_.running)return false;if(logNeedsRotation(file_.size(),length+12)&&!rotate())return false;stats_.used=LittleFS.usedBytes();if(!logHasReserve(stats_.total,stats_.used)){stats_.running=false;file_.close();return false;}if(length>512){stats_.dropped++;return false;}uint8_t record[524];size_t n=encodeLogRecord(type,1,payload,length,record,sizeof record);if(!n||file_.write(record,n)!=n){stats_.running=false;return false;}stats_.records++;stats_.bytes+=n;uint32_t elapsed=(millis()-startedMs_)/1000;if(elapsed){stats_.bytesPerSecond=stats_.bytes/elapsed;if(stats_.bytesPerSecond)stats_.estimatedMinutesRemaining=(stats_.total-stats_.used)/stats_.bytesPerSecond/60;}return true;}
bool BinaryLogger::rotate(){LoggerLock lock(mutex_);if(file_){file_.flush();file_.close();}return open();}void BinaryLogger::flush(){LoggerLock lock(mutex_);if(file_)file_.flush();}
#endif
}
