#pragma once
#ifdef ARDUINO
#include <WebServer.h>
#include "BmsSnapshot.h"
#include "BinaryLogger.h"
#include "RtcClock.h"
namespace dms {class WebPortal{public:bool begin(const char*role,uint8_t channel,const char*password,SnapshotStore&,BinaryLogger&,RtcClock&);void handle();const char*ssid()const{return ssid_;}private:WebServer server_{80};SnapshotStore*store_=nullptr;BinaryLogger*logger_=nullptr;RtcClock*rtc_=nullptr;const char*role_=nullptr;char ssid_[40]{};void root();void status();void live();void logs();void fileDownload();void csv();void setTime();void rotate();void removeOne();void eraseAll();bool safe(const String&)const;};}
#endif
