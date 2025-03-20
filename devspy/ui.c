#include "pch.h"
#include "resource.h"
#include "ui.h"

static const wchar_t* app_class_name = L"DevSpyMainWindow";
static ATOM app_class = 0;
static HWND app_window = NULL;
static HACCEL app_accel = NULL;
static HINSTANCE app_instance = NULL;
static app_data_t* app_data = NULL;

static LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_NCDESTROY:
		app_window = NULL;
		break;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

static void uninit_ui(void)
{
	if (app_window)
	{
		DestroyWindow(app_window);
	}

	if (app_class)
	{
		UnregisterClass(app_class_name, app_instance);
		app_class = 0;
	}

	if (app_accel)
	{
		DestroyAcceleratorTable(app_accel);
		app_accel = NULL;
	}

	app_data = NULL;
	app_instance = NULL;
}

BOOL init_ui(HINSTANCE instance, app_data_t* data)
{
	BOOL result = TRUE;

	app_instance = instance;
	app_data = data;

	app_accel = LoadAccelerators(instance, MAKEINTRESOURCE(IDR_APP_ACCELERATOR));
	if (!app_accel)
	{
		result = FALSE;
		goto cleanup;
	}

	WNDCLASS wc =
	{
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = MainWindowProc,
		.hInstance = instance,
		.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_MAIN)),
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
		.lpszMenuName = NULL,
		.lpszClassName = app_class_name,
	};

	app_class = RegisterClass(&wc);
	if (!app_class)
	{
		result = FALSE;
		goto cleanup;
	}

	app_window = CreateWindowEx(
		0, // exstyle
		app_class_name,
		L"DevSpy",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, // parent
		NULL, // menu
		instance,
		NULL); // param
	if (!app_window)
	{
		result = FALSE;
		goto cleanup;
	}

	ShowWindow(app_window, SW_SHOWDEFAULT);

cleanup:
	if (!result)
	{
		uninit_ui();
	}

	return result;
}

int run_ui(void)
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, app_accel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	uninit_ui();

	return (int)msg.wParam;
}
