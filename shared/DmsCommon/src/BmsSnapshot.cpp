#include "BmsSnapshot.h"
namespace dms {SnapshotStore::SnapshotStore(){
#ifdef ARDUINO
mutex_=xSemaphoreCreateMutex();
#endif
}SnapshotStore::~SnapshotStore(){
#ifdef ARDUINO
if(mutex_)vSemaphoreDelete(mutex_);
#endif
}void SnapshotStore::publish(const BmsSnapshot&s){
#ifdef ARDUINO
xSemaphoreTake(mutex_,portMAX_DELAY);
#else
std::lock_guard<std::mutex> lock(mutex_);
#endif
uint8_t n=active_^1;slots_[n]=s;active_=n;valid_=true;
#ifdef ARDUINO
xSemaphoreGive(mutex_);
#endif
}bool SnapshotStore::latest(BmsSnapshot&o)const{
#ifdef ARDUINO
xSemaphoreTake(mutex_,portMAX_DELAY);
#else
std::lock_guard<std::mutex> lock(mutex_);
#endif
bool ok=valid_;if(ok)o=slots_[active_];
#ifdef ARDUINO
xSemaphoreGive(mutex_);
#endif
return ok;}bool SnapshotStore::bySequence(uint32_t q,BmsSnapshot&o)const{
#ifdef ARDUINO
xSemaphoreTake(mutex_,portMAX_DELAY);
#else
std::lock_guard<std::mutex> lock(mutex_);
#endif
bool ok=false;if(valid_)for(const auto&s:slots_)if(s.sequence==q){o=s;ok=true;break;}
#ifdef ARDUINO
xSemaphoreGive(mutex_);
#endif
return ok;}}
