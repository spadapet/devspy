#pragma once

#ifdef DEVSPY_HOOK_DLL
#define DEVSPY_API __declspec(dllexport)
#else
#define DEVSPY_API __declspec(dllimport)
#endif

#ifdef _WIN64
#define UNIQUE_NAME_SUFFIX L"_64_PS"
#else
#define UNIQUE_NAME_SUFFIX L"_32_PS"
#endif

#define APP_MUTEX_NAME L"DevSpyMutex" UNIQUE_NAME_SUFFIX
#define DATA_FILE_NAME L"DevSpyDataFile" UNIQUE_NAME_SUFFIX
#define DATA_MUTEX_NAME L"DevSpyDataMutex" UNIQUE_NAME_SUFFIX
#define DATA_EVENT_NAME L"DevSpyDataEvent" UNIQUE_NAME_SUFFIX
#define LOG_ENTRY_COUNT 16384 // 1MB of entries (64 bytes each)
#define DATA_ENTRY_COUNT 4096 // 1MB of data (256 bytes each)
#define LOG_ALIGN __declspec(align(64))

typedef LOG_ALIGN struct log_entry_t
{
    UINT index; // 4
    UINT data_index; // 4
    UINT data_count; // 4
    UINT message; // 4
    HWND hwnd; // 4/8
    WPARAM wp; // 4/8
    LPARAM lp; // 4/8
    DWORD process_id; // 4
    DWORD thread_id; // 4
    // Total: 36/48
} log_entry_t;

typedef LOG_ALIGN struct log_data_t
{
	UINT index;
	UINT padding;
	BYTE data[248];
} log_data_t;

typedef LOG_ALIGN struct app_data_t
{
	DWORD app_process_id;
	UINT active;
	UINT write_index;
	UINT read_index;
    // 48 bytes of padding
	log_entry_t entries[LOG_ENTRY_COUNT];
	log_data_t data[DATA_ENTRY_COUNT];
} app_data_t;

DEVSPY_API void start_hook(void);
DEVSPY_API void stop_hook(void);

#undef LOG_ALIGN
#undef DEVSPY_API
