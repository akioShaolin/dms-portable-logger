#pragma once
#include "JbdCodec.h"
#include <stdint.h>
#ifdef ARDUINO
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#else
#include <mutex>
#endif
namespace dms {
constexpr uint16_t SNAPSHOT_SCHEMA=1;
enum Quality:uint32_t{BASIC_VALID=1,CELLS_VALID=2,HARDWARE_VALID=4,RTC_VALID=8,BMS_ONLINE=16,STALE=32,PROTECTION_ACTIVE=64,UNEXPECTED_CELL_COUNT=128,EXTRA_PRESERVED=256,LINK_ONLINE=512,CLOCK_UNSET=1024};
struct BmsSnapshot{
  uint16_t schema=SNAPSHOT_SCHEMA;uint32_t bootId=0,sequence=0;uint64_t timestampUtcMs=0;uint32_t captureMonotonicMs=0,quality=0,pollDurationUs=0;
  BasicInfo basic{};CellInfo cells{};char hardware[32]{};JbdFrame raw03{},raw04{},raw05{};
  uint32_t hardwareCaptureMs=0,polls=0,timeouts=0,checksumErrors=0,discardedFrames=0,linkErrors=0,droppedLogs=0;
  uint32_t panelRequests=0,proxyRequests=0,cacheResponses=0,writeBlocked=0,lastPanelLatencyMs=0,sourceAgeMs=0;uint8_t lastPanelCommand=0,lastPanelFlags=0;
};
class SnapshotStore{public:SnapshotStore();~SnapshotStore();void publish(const BmsSnapshot&);bool latest(BmsSnapshot&)const;bool bySequence(uint32_t,BmsSnapshot&)const;private:BmsSnapshot slots_[2]{};uint8_t active_=0;bool valid_=false;
#ifdef ARDUINO
mutable SemaphoreHandle_t mutex_=nullptr;
#else
mutable std::mutex mutex_;
#endif
};
}
