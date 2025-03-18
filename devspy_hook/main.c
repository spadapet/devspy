#include "pch.h"
#include "api.h"

static HINSTANCE module_handle = NULL;
static HANDLE app_mutex = NULL;
static HANDLE app_file = NULL;
static app_data_t* app_data = NULL;
static HHOOK call_wnd_proc_hook_handle = NULL;
static HHOOK get_message_hook_handle = NULL;

static BOOL init_dll(HINSTANCE module)
{
	module_handle = module;
	DisableThreadLibraryCalls(module);

	app_mutex = OpenMutex(SYNCHRONIZE, FALSE, APP_MUTEX_NAME);
	if (!app_mutex)
	{
		// Running within the main app
		return TRUE;
	}

	app_file = OpenFileMapping(FILE_MAP_WRITE, FALSE, DATA_FILE_NAME);
	if (!app_file)
	{
		return FALSE;
	}

	app_data = MapViewOfFile(app_file, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(app_data_t));
	if (!app_data)
	{
		return FALSE;
	}
}

static void uninit_dll()
{
	module_handle = NULL;

	if (app_data)
	{
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
		CloseHandle(app_mutex);
		app_mutex = NULL;
	}
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, void* reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		return init_dll(module);

	case DLL_PROCESS_DETACH:
		uninit_dll();
		break;
	}

	return TRUE;
}

static LRESULT CALLBACK call_wnd_proc_hook(int code, WPARAM wp, LPARAM lp)
{
	if (code >= 0)
	{
		CWPRETSTRUCT* p = (CWPRETSTRUCT*)lp;

		DWORD process_id = 0;
		DWORD thread_id = GetWindowThreadProcessId(p->hwnd, &process_id);

		wchar_t buffer[256];
		_snwprintf_s(buffer, 256, 256, L"SEND: Process ID: %lu, Thread ID: %lu, HWND: %p\n", process_id, thread_id, p->hwnd);
		OutputDebugString(buffer);
	}

	return CallNextHookEx(call_wnd_proc_hook_handle, code, wp, lp);
}

static LRESULT CALLBACK get_message_hook(int code, WPARAM wp, LPARAM lp)
{
	if (code >= 0)
	{
		MSG* p = (MSG*)lp;

		DWORD process_id = 0;
		DWORD thread_id = GetWindowThreadProcessId(p->hwnd, &process_id);

		wchar_t buffer[256];
		_snwprintf_s(buffer, 256, 256, L"POST: Process ID: %lu, Thread ID: %lu, HWND: %p\n", process_id, thread_id, p->hwnd);
		OutputDebugString(buffer);
	}

	return CallNextHookEx(get_message_hook_handle, code, wp, lp);
}

void start_hook(void)
{
	if (!call_wnd_proc_hook_handle)
	{
		call_wnd_proc_hook_handle = SetWindowsHookEx(WH_CALLWNDPROC, call_wnd_proc_hook, module_handle, 0);
	}

	if (!get_message_hook_handle)
	{
		get_message_hook_handle = SetWindowsHookEx(WH_GETMESSAGE, get_message_hook, module_handle, 0);
	}
}

void stop_hook(void)
{
	if (get_message_hook_handle)
	{
		UnhookWindowsHookEx(get_message_hook_handle);
		get_message_hook_handle = NULL;
	}

	if (call_wnd_proc_hook_handle)
	{
		UnhookWindowsHookEx(call_wnd_proc_hook_handle);
		call_wnd_proc_hook_handle = NULL;
	}
}
