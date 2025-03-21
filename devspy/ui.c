#include "pch.h"
#include "resource.h"
#include "ui.h"

enum
{
	ID_HWND_NULL,
	ID_HWND_LIST,
};

static HDWP layout_children(HWND root_hwnd, HDWP defer)
{
	const uint32_t swp_flags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER;

	RECT client;
	if (!GetClientRect(root_hwnd, &client))
	{
		return defer;
	}

	for (HWND hwnd = GetWindow(root_hwnd, GW_CHILD); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT))
	{
		switch (GetDlgCtrlID(hwnd))
		{
		case ID_HWND_LIST:
			defer = DeferWindowPos(defer, hwnd, NULL, 0, 0, client.right, client.bottom, swp_flags);
			break;
		}
	}

	return defer;
}

static void layout_root(HWND hwnd)
{
	HDWP defer = BeginDeferWindowPos(32);
	defer = layout_children(hwnd, defer);
	EndDeferWindowPos(defer);
}

static LRESULT CALLBACK main_window_proc(HWND hwnd, uint32_t msg, WPARAM wp, LPARAM lp)
{
	static app_data_t* app_data = NULL;

	switch (msg)
	{
	case WM_CREATE:
	{
		const CREATESTRUCT* cs = (CREATESTRUCT*)lp;
		app_data = (app_data_t*)cs->lpCreateParams;
	} break;

	case WM_SIZE:
		if (wp != SIZE_MINIMIZED)
		{
			layout_root(hwnd);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_NCDESTROY:
		app_data = NULL;
		break;

	case WM_DPICHANGED:
	{
		const RECT* rect = (const RECT*)lp;
		SetWindowPos(hwnd, NULL,
			rect->left,
			rect->top,
			rect->right - rect->left,
			rect->bottom - rect->top,
			SWP_NOZORDER | SWP_NOACTIVATE);
	} break;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

bool init_ui(HINSTANCE instance, app_data_t* data)
{
	bool result = true;
	HWND main_window = NULL;

	const INITCOMMONCONTROLSEX cc =
	{
		.dwSize = sizeof(INITCOMMONCONTROLSEX),
		.dwICC = 0xFFFF, // all common controls
	};

	if (!InitCommonControlsEx(&cc))
	{
		result = false;
		goto cleanup;
	}

	const WNDCLASSEX wc =
	{
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = main_window_proc,
		.hInstance = instance,
		.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_MAIN)),
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
		.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU),
		.lpszClassName = L"DevSpyMainWindow",
		.hIconSm = LoadIcon(instance, MAKEINTRESOURCE(IDI_MAIN)),
	};

	if (!RegisterClassEx(&wc))
	{
		result = false;
		goto cleanup;
	}

	main_window = CreateWindowEx(
		0, // exstyle
		wc.lpszClassName,
		L"DevSpy",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, // parent
		NULL, // menu
		instance,
		data); // param

	if (!main_window)
	{
		result = false;
		goto cleanup;
	}

	HWND list_window = CreateWindowEx(
		0, // exstyle
		WC_LISTVIEW,
		L"",
		WS_CHILD | WS_VISIBLE | LVS_REPORT,
		0, 0, 0, 0, // position
		main_window,
		(HMENU)ID_HWND_LIST,
		instance,
		NULL); // param

	if (!list_window)
	{
		result = false;
		goto cleanup;
	}

	layout_root(main_window);
	ShowWindow(main_window, SW_SHOWDEFAULT);

cleanup:
	if (!result && main_window)
	{
		DestroyWindow(main_window);
	}

	return result;
}

int run_ui(HINSTANCE instance)
{
	HACCEL accel = LoadAccelerators(instance, MAKEINTRESOURCE(IDR_APP_ACCELERATOR));

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, accel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DestroyAcceleratorTable(accel);

	return (int)msg.wParam;
}
