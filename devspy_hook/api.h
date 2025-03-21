#pragma once

#ifdef DEVSPY_HOOK_DLL
#define DEVSPY_API __declspec(dllexport)
#else
#define DEVSPY_API __declspec(dllimport)
#endif

#ifdef _WIN64
#define PROCESS_BITS_SZ L"64"
#else
#define PROCESS_BITS_SZ L"32"
#endif

#define APP_EXE_NAME L"DevSpy" PROCESS_BITS_SZ L".exe"
#define UNIQUE_NAME_SUFFIX L"_" PROCESS_BITS_SZ L"_PS"
#define APP_MUTEX_NAME L"DevSpyMutex" UNIQUE_NAME_SUFFIX
#define DATA_FILE_NAME L"DevSpyDataFile" UNIQUE_NAME_SUFFIX
#define DATA_MUTEX_NAME L"DevSpyDataMutex" UNIQUE_NAME_SUFFIX
#define DATA_EVENT_NAME L"DevSpyDataEvent" UNIQUE_NAME_SUFFIX
#define LOG_ENTRY_COUNT 16384 // 1MB of entries (64 bytes each)
#define DATA_ENTRY_COUNT 4096 // 1MB of data (256 bytes each)
#define LOG_ALIGN __declspec(align(64))

typedef LOG_ALIGN struct log_entry_t
{
	uint32_t index; // 4
	uint32_t data_index; // 4
	uint32_t data_count; // 4
	uint32_t process_id; // 4
	uint32_t thread_id; // 4
	uint32_t message; // 4
	HWND hwnd; // 4/8
	WPARAM wp; // 4/8
	LPARAM lp; // 4/8
	// Total: 36/48
} log_entry_t;

typedef LOG_ALIGN struct log_data_t
{
	uint32_t index;
	uint32_t padding;
	uint8_t data[248];
} log_data_t;

typedef LOG_ALIGN struct app_data_t
{
	uint32_t app_process_id;
	uint32_t active;
	uint32_t write_index;
	uint32_t read_index;
	uint8_t padding[48];
	log_entry_t entries[LOG_ENTRY_COUNT];
	log_data_t data[DATA_ENTRY_COUNT];
} app_data_t;

DEVSPY_API void start_hook(void);
DEVSPY_API void stop_hook(void);

#undef LOG_ALIGN
#undef DEVSPY_API
