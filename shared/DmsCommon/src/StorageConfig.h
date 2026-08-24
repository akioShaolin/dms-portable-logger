#pragma once
#include <stddef.h>
#include <stdint.h>

namespace dms {
constexpr const char *LITTLEFS_PARTITION_LABEL = "littlefs";
constexpr const char *LITTLEFS_BASE_PATH = "/littlefs";
constexpr uint8_t LITTLEFS_MAX_OPEN_FILES = 10;
constexpr uint32_t RAW_FRAME_SAMPLE_MS = 60000;

enum class StorageError : uint8_t {
  NONE,
  PARTITION_NOT_FOUND,
  MOUNT_FAILED,
  NOT_FORMATTED,
  LOGGER_OPEN_FAILED,
  HEADER_WRITE_FAILED,
  WRITE_FAILED,
  RESERVE_REACHED,
  MANUALLY_STOPPED
};

inline const char *storageErrorCode(StorageError error) {
  switch (error) {
    case StorageError::NONE: return "none";
    case StorageError::PARTITION_NOT_FOUND: return "partition_not_found";
    case StorageError::MOUNT_FAILED: return "littlefs_mount_failed";
    case StorageError::NOT_FORMATTED: return "littlefs_not_formatted";
    case StorageError::LOGGER_OPEN_FAILED: return "logger_open_failed";
    case StorageError::HEADER_WRITE_FAILED: return "header_write_failed";
    case StorageError::WRITE_FAILED: return "write_failed";
    case StorageError::RESERVE_REACHED: return "reserve_reached";
    case StorageError::MANUALLY_STOPPED: return "manually_stopped";
  }
  return "unknown";
}
inline int storageLogsHttpStatus(bool mounted) { return mounted ? 200 : 503; }
inline StorageError loggerInitializationError(bool mounted, bool opened, bool headerWritten, bool flushedAndListed) {
  if (!mounted) return StorageError::MOUNT_FAILED;
  if (!opened) return StorageError::LOGGER_OPEN_FAILED;
  if (!headerWritten || !flushedAndListed) return StorageError::HEADER_WRITE_FAILED;
  return StorageError::NONE;
}
inline bool activeLogMatches(const char *candidate, const char *active, bool running) {
  if (!running || !candidate || !active) return false;
  size_t i=0;while(candidate[i]&&active[i]&&candidate[i]==active[i])i++;
  return candidate[i]==0&&active[i]==0;
}

inline bool safeLogBasename(const char *name) {
  if (!name || !*name) return false;
  size_t length = 0;
  for (const unsigned char *p = reinterpret_cast<const unsigned char *>(name); *p; ++p, ++length)
    if (*p < 0x20 || *p == 0x7f || *p == '/' || *p == '\\' || *p == '"') return false;
  if (length < 15 || (name[0] != 'M' && name[0] != 'S') || name[1] != '-') return false;
  const char suffix[] = ".dmslog";
  for (size_t i = 0; i < sizeof(suffix); ++i)
    if (name[length - (sizeof(suffix) - 1) + i] != suffix[i]) return false;
  size_t hyphens=0;for(size_t i=0;i<length-7;i++){char c=name[i];if(c=='-')hyphens++;else if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')))return false;}
  if(hyphens<2)return false;size_t digits=length-10;if(name[digits-1]!='-')return false;
  return name[digits]>='0'&&name[digits]<='9'&&name[digits+1]>='0'&&name[digits+1]<='9'&&name[digits+2]>='0'&&name[digits+2]<='9';
}

inline bool normalizeLogName(const char*input,char*basename,size_t basenameSize,char*path,size_t pathSize){if(!input||!basename||!path)return false;const char*p=input;if(*p=='/')p++;if(!safeLogBasename(p))return false;size_t n=0;while(p[n])n++;if(n+1>basenameSize||n+2>pathSize)return false;for(size_t i=0;i<=n;i++)basename[i]=p[i];path[0]='/';for(size_t i=0;i<=n;i++)path[i+1]=p[i];return true;}
struct DeleteSummary{uint32_t candidates=0,removed=0,skippedActive=0,rejected=0,failed=0;};
inline bool deletionSucceeded(const DeleteSummary&s){return s.failed==0&&!(s.candidates&&s.removed==0);}

struct RawFramePolicy {
  uint8_t bytes[128]{};
  uint16_t length = 0;
  uint32_t lastStoredMs = 0;
  bool initialized = false;
};

inline bool shouldStoreRaw(RawFramePolicy &state, const uint8_t *bytes, uint16_t length,
                           uint32_t nowMs, bool force = false) {
  bool changed = !state.initialized || state.length != length;
  if (!changed && length) {
    for (uint16_t i = 0; i < length; ++i) if (state.bytes[i] != bytes[i]) { changed = true; break; }
  }
  if (!force && !changed && uint32_t(nowMs - state.lastStoredMs) < RAW_FRAME_SAMPLE_MS) return false;
  state.length = length > sizeof(state.bytes) ? sizeof(state.bytes) : length;
  for (uint16_t i = 0; i < state.length; ++i) state.bytes[i] = bytes[i];
  state.lastStoredMs = nowMs;
  state.initialized = true;
  return true;
}
}
