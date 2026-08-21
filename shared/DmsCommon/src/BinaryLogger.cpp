#include "BinaryLogger.h"
#include "DmsLink.h"
#include <string.h>
#ifdef ARDUINO
#include <LittleFS.h>
#include <stdio.h>
class LoggerLock{public:explicit LoggerLock(SemaphoreHandle_t h):h_(h){if(h_)xSemaphoreTakeRecursive(h_,portMAX_DELAY);}~LoggerLock(){if(h_)xSemaphoreGiveRecursive(h_);}private:SemaphoreHandle_t h_;};
#endif
namespace dms {
size_t encodeLogRecord(LogType type,uint8_t version,const uint8_t*payload,uint32_t length,uint8_t*out,size_t capacity){if(length>65536||capacity<size_t(length)+12)return 0;uint8_t*p=out;put16(p,LOG_SYNC);*p++=uint8_t(type);*p++=version;put32(p,length);if(length)memcpy(p,payload,length);p+=length;put32(p,crc32(out,8+length));return p-out;}
bool logNeedsRotation(uint64_t current,uint32_t next){return current&&current+next>LOG_ROTATE_BYTES;}bool logHasReserve(uint64_t total,uint64_t used){return total>used&&total-used>=LOG_RESERVE_BYTES;}
bool validLogHeader(const uint8_t*h,size_t n){if(!h||n!=LOG_HEADER_SIZE||memcmp(h,"DMSLOG2",7)||h[7]!=2)return false;const uint8_t*p=h+60;return get32(p)==crc32(h,60);}
#ifdef ARDUINO
void BinaryLogger::setStorageState(bool found,bool mounted,StorageError error){stats_.partitionFound=found;stats_.mounted=mounted;stats_.running=false;stats_.error=error;stats_.total=mounted?LittleFS.totalBytes():0;stats_.used=mounted?LittleFS.usedBytes():0;}
void BinaryLogger::fail(StorageError e){stats_.running=false;stats_.error=e;if(file_)file_.close();}
bool BinaryLogger::begin(fs::FS&fs,uint8_t role,uint32_t boot,uint32_t session,const char*device,const char*firmware,const char*gitSha,uint8_t clockQuality,uint64_t startEpochMs){if(!stats_.mounted)return false;if(!mutex_)mutex_=xSemaphoreCreateRecursiveMutex();if(!mutex_){fail(StorageError::LOGGER_OPEN_FAILED);return false;}fs_=&fs;role_=role;boot_=boot;session_=session;clockQuality_=clockQuality;startEpochMs_=startEpochMs;startedMs_=millis();strncpy(device_,device,sizeof(device_)-1);strncpy(firmware_,firmware,sizeof(firmware_)-1);strncpy(gitSha_,gitSha,sizeof(gitSha_)-1);return open();}
bool BinaryLogger::open(){snprintf(name_,sizeof name_,"/%c-%08lx-%08lx-%03lu.dmslog",role_?'S':'M',(unsigned long)boot_,(unsigned long)session_,(unsigned long)index_++);file_=fs_->open(name_,FILE_WRITE);if(!file_){fail(loggerInitializationError(true,false,false,false));return false;}uint8_t h[LOG_HEADER_SIZE]{};memcpy(h,"DMSLOG2",7);h[7]=2;h[8]=role_;h[9]=clockQuality_;uint8_t*p=h+10;put16(p,1);put32(p,boot_);put32(p,session_);put64(p,startEpochMs_);memcpy(h+28,device_,12);memcpy(h+40,firmware_,8);memcpy(h+48,gitSha_,12);p=h+60;put32(p,crc32(h,60));bool written=file_.write(h,sizeof h)==sizeof h;if(!written){fail(loggerInitializationError(true,true,false,false));return false;}file_.flush();stats_.lastFlushMs=millis();bool stable=file_.size()>=LOG_HEADER_SIZE&&fs_->exists(name_);if(!stable){fail(loggerInitializationError(true,true,true,false));return false;}stats_.bytes+=sizeof h;stats_.running=true;stats_.error=loggerInitializationError(true,true,true,true);stats_.total=LittleFS.totalBytes();stats_.used=LittleFS.usedBytes();return true;}
bool BinaryLogger::append(LogType type,const uint8_t*payload,uint32_t length){LoggerLock lock(mutex_);if(!stats_.running){stats_.dropped++;return false;}if(logNeedsRotation(file_.size(),length+12)&&!rotate())return false;stats_.used=LittleFS.usedBytes();if(!logHasReserve(stats_.total,stats_.used)){fail(StorageError::RESERVE_REACHED);return false;}if(length>512){stats_.dropped++;return false;}uint8_t record[524];size_t n=encodeLogRecord(type,1,payload,length,record,sizeof record);if(!n||file_.write(record,n)!=n){stats_.dropped++;fail(StorageError::WRITE_FAILED);return false;}stats_.records++;stats_.bytes+=n;stats_.lastWriteMs=millis();uint32_t elapsed=(millis()-startedMs_)/1000;if(elapsed){stats_.bytesPerSecond=stats_.bytes/elapsed;if(stats_.bytesPerSecond)stats_.estimatedMinutesRemaining=(stats_.total-stats_.used)/stats_.bytesPerSecond/60;}return true;}
bool BinaryLogger::rotate(){LoggerLock lock(mutex_);if(!stats_.mounted||!stats_.running)return false;if(file_){file_.flush();stats_.lastFlushMs=millis();file_.close();}stats_.rotationCount++;return open();}
bool BinaryLogger::flush(){LoggerLock lock(mutex_);if(!stats_.running||!file_)return false;file_.flush();stats_.lastFlushMs=millis();return file_.size()>=LOG_HEADER_SIZE;}
#endif
}
