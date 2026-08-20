#pragma once
#include <stdint.h>
namespace config {constexpr uint32_t LINK_BAUD=230400,JBD_BAUD=9600,POLL_MS=1000,RESPONSE_TIMEOUT_MS=250,FRESH_AGE_MS=2500,STALE_FALLBACK_MS=5000;constexpr uint8_t EXPECTED_CELL_COUNT=24,WIFI_CHANNEL=1;constexpr char AP_PASSWORD[]="dmslogger";}
