#include "BmsSnapshot.h"
#ifdef ARDUINO
#include <freertos/FreeRTOS.h>
static portMUX_TYPE snapshotMux=portMUX_INITIALIZER_UNLOCKED;
#endif
namespace dms { void SnapshotStore::publish(const BmsSnapshot&s){uint8_t n=active_^1;
#ifdef ARDUINO
portENTER_CRITICAL(&snapshotMux);
#endif
slots_[n]=s;active_=n;valid_=true;
#ifdef ARDUINO
portEXIT_CRITICAL(&snapshotMux);
#endif
} bool SnapshotStore::latest(BmsSnapshot&o)const{if(!valid_)return false;o=slots_[active_];return true;}bool SnapshotStore::bySequence(uint32_t q,BmsSnapshot&o)const{if(!valid_)return false;for(const auto&s:slots_)if(s.sequence==q){o=s;return true;}return false;} }
