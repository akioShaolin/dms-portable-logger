#pragma once
#include <stdint.h>
#if __has_include("secrets.h")
#include "secrets.h"
#else
#define DMS_AP_PASSWORD "dmslogger"
#endif
namespace config {constexpr uint32_t LINK_BAUD=230400,JBD_BAUD=9600,PANEL_TIMEOUT_MS=250,FRESH_AGE_MS=2500,STALE_FALLBACK_MS=5000,BURST_LEASE_MS=400;constexpr uint8_t EXPECTED_CELL_COUNT=24,WIFI_CHANNEL=6;constexpr const char*AP_PASSWORD=DMS_AP_PASSWORD;}
