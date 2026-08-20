#pragma once
#include "BmsSnapshot.h"
#include <stddef.h>
#include <stdint.h>
namespace dms {constexpr uint16_t REGISTER_COUNT=256;class RegisterBank{public:void update(const BmsSnapshot&,uint32_t age,uint8_t role);bool read(uint16_t address,uint16_t count,uint16_t*out,size_t capacity)const;const uint16_t*data()const{return regs_;}private:uint16_t regs_[REGISTER_COUNT]{};};}
