#include "pch.h"
#include "ui.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

static HANDLE app_mutex = NULL;
static HANDLE app_file = NULL;
static app_data_t* app_data = NULL;

static bool init_app(void)
{
	app_mutex = CreateMutex(NULL, true, APP_MUTEX_NAME);
	if (!app_mutex || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return false;
	}

	app_file = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(app_data_t), DATA_FILE_NAME);
	if (!app_file)
	{
		return false;
	}

	app_data = MapViewOfFile(app_file, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(app_data_t));
	if (!app_data)
	{
		return false;
	}

	ZeroMemory(app_data, sizeof(app_data_t));
	app_data->app_process_id = GetCurrentProcessId();

	//start_hook();

	return true;
}

static void uninit_app(void)
{
	stop_hook();

	if (app_data)
	{
		app_data->active = 0;
		app_data->app_process_id = 0;
		UnmapViewOfFile(app_data);
		app_data = NULL;
	}

	if (app_file)
	{
		CloseHandle(app_file);
		app_file = NULL;
	}

	if (app_mutex)
	{
		ReleaseMutex(app_mutex);
		CloseHandle(app_mutex);
		app_mutex = NULL;
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previous_instance, _In_ wchar_t* command_line, _In_ int command_show)
{
	int result = (init_app() && init_ui(instance, app_data)) ? run_ui(instance) : 1;
	uninit_app();
	return result;
}
