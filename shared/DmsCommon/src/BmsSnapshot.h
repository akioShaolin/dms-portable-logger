#pragma once
#include "JbdCodec.h"
#include <stdint.h>
namespace dms {
constexpr uint16_t SNAPSHOT_SCHEMA=1; enum Quality:uint32_t{BASIC_VALID=1,CELLS_VALID=2,HARDWARE_VALID=4,RTC_VALID=8,BMS_ONLINE=16,STALE=32,PROTECTION_ACTIVE=64,UNEXPECTED_CELL_COUNT=128,EXTRA_PRESERVED=256};
struct BmsSnapshot { uint16_t schema=SNAPSHOT_SCHEMA;uint32_t bootId=0,sequence=0;uint64_t timestampUtcMs=0;uint32_t captureMonotonicMs=0,quality=0,pollDurationUs=0;BasicInfo basic{};CellInfo cells{};JbdFrame raw03{},raw04{},raw05{};uint32_t timeouts=0,checksumErrors=0,discardedFrames=0; };
class SnapshotStore { public: void publish(const BmsSnapshot&s); bool latest(BmsSnapshot&out)const; bool bySequence(uint32_t seq,BmsSnapshot&out)const; private: BmsSnapshot slots_[2]{};volatile uint8_t active_=0;volatile bool valid_=false; };
}
