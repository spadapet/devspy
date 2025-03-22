#include "pch.h"
#include "resource.h"
#include "ui.h"

typedef enum window_id
{
	ID_HWND_NULL,
	ID_HWND_PROCESSES_LIST,
	ID_HWND_WINDOWS_LIST,
	ID_HWND_MESSAGES_LIST,
	ID_HWND_INFO,
} window_id;

typedef enum app_mode_t
{
	APP_MODE_PROCESSES,
	APP_MODE_WINDOWS,
	APP_MODE_MESSAGES,
} app_mode_t;

typedef enum list_columns
{
	LIST_COL_PROCESS_ID, // all modes
	LIST_COL_THREAD_ID, // windows and messages
	LIST_COL_HWND, // windows and messages
	LIST_COL_MESSAGE, // messages
	LIST_COL_WPARAM, // messages
	LIST_COL_LPARAM, // messages
	LIST_COL_CLASS, // windows
	LIST_COL_TEXT, // windows
	LIST_COL_NAME, // processes
	LIST_COL_PATH, // processes
} list_columns;

static float list_column_widths[] = // logical units, not pixels
{
	[LIST_COL_PROCESS_ID] = 100,
	[LIST_COL_THREAD_ID] = 100,
	[LIST_COL_HWND] = 100,
	[LIST_COL_MESSAGE] = 100,
	[LIST_COL_WPARAM] = 100,
	[LIST_COL_LPARAM] = 100,
	[LIST_COL_CLASS] = 100,
	[LIST_COL_TEXT] = 100,
	[LIST_COL_NAME] = 100,
	[LIST_COL_PATH] = 100,
};

static app_mode_t app_mode = APP_MODE_PROCESSES;
static float main_splitter_percent = 0.75f;

static int to_pixels(int value, int dpi)
{
	return MulDiv(value, dpi, 96);
}

static int from_pixels(int value, int dpi)
{
	return MulDiv(value, 96, dpi);
}

bool get_layout_rects(HWND main_window, RECT* client, RECT* splitter_rect)
{
	if (!GetClientRect(main_window, client) || IsRectEmpty(client))
	{
		return false;
	}

	const int dpi = GetDpiForWindow(main_window);
	const int splitter_width = to_pixels(8, dpi);

	main_splitter_percent = max(0.1f, min(main_splitter_percent, 0.9f));
	splitter_rect->left = (int)(client->right * main_splitter_percent);

	splitter_rect->right = splitter_rect->left + splitter_width;
	splitter_rect->top = 0;
	splitter_rect->bottom = client->bottom;

	return true;
}

static HDWP layout_children(HWND main_window, HDWP defer)
{
	const uint32_t swp_flags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER;

	RECT client, splitter_rect;
	if (!get_layout_rects(main_window, &client, &splitter_rect))
	{
		return defer;
	}

	for (HWND hwnd = GetWindow(main_window, GW_CHILD); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT))
	{
		int id = GetDlgCtrlID(hwnd);
		switch (id)
		{
		case ID_HWND_PROCESSES_LIST:
		case ID_HWND_WINDOWS_LIST:
		case ID_HWND_MESSAGES_LIST:
		{
			bool visible =
				(id == ID_HWND_PROCESSES_LIST && app_mode == APP_MODE_PROCESSES) ||
				(id == ID_HWND_WINDOWS_LIST && app_mode == APP_MODE_WINDOWS) ||
				(id == ID_HWND_MESSAGES_LIST && app_mode == APP_MODE_MESSAGES);
			defer = DeferWindowPos(defer, hwnd, NULL, 0, 0, splitter_rect.left, client.bottom, swp_flags |
				(visible ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
		} break;

		case ID_HWND_INFO:
			defer = DeferWindowPos(defer, hwnd, NULL, splitter_rect.right, 0, client.right - splitter_rect.right, client.bottom, swp_flags);
			break;
		}
	}

	return defer;
}

static void main_window_layout(HWND main_window)
{
	HDWP defer = BeginDeferWindowPos(8);
	defer = layout_children(main_window, defer);
	EndDeferWindowPos(defer);
}

static void main_window_update_title(HWND main_window)
{
	switch (app_mode)
	{
	case APP_MODE_PROCESSES:
		SetWindowText(main_window, L"DevSpy - Processes");
		break;

	case APP_MODE_WINDOWS:
		SetWindowText(main_window, L"DevSpy - Windows");
		break;

	case APP_MODE_MESSAGES:
		SetWindowText(main_window, L"DevSpy - Messages");
		break;

	default:
		SetWindowText(main_window, L"DevSpy");
		break;
	}
}

static void main_window_update_mode(HWND main_window)
{
	main_window_layout(main_window);
	main_window_update_title(main_window);
}

static void update_list_column_widths(HWND list_hwnd)
{
	int dpi = GetDpiForWindow(list_hwnd);

	HWND header_hwnd = ListView_GetHeader(list_hwnd);
	int count = Header_GetItemCount(header_hwnd);

	LVCOLUMN col = { .mask = LVCF_SUBITEM };

	for (int i = 0; i < count; i++)
	{
		ListView_GetColumn(list_hwnd, i, &col);
		ListView_SetColumnWidth(list_hwnd, i, (int)(list_column_widths[col.iSubItem] * dpi / 96.0f));
	}
}

static void create_list_columns(app_mode_t mode, HWND list_hwnd)
{
	int dpi = GetDpiForWindow(list_hwnd);

	LVCOLUMN col = { 0 };
	col.mask = LVCF_TEXT | LVCF_SUBITEM;

	switch (mode)
	{
	case APP_MODE_PROCESSES:
		col.pszText = L"ID";
		col.iSubItem = LIST_COL_PROCESS_ID;
		ListView_InsertColumn(list_hwnd, 0, &col);

		col.pszText = L"Name";
		col.iSubItem = LIST_COL_NAME;
		ListView_InsertColumn(list_hwnd, 1, &col);
		break;

	case APP_MODE_WINDOWS:
		break;

	case APP_MODE_MESSAGES:
		break;
	}

	update_list_column_widths(list_hwnd);
}

static bool main_window_init(HWND main_window, HINSTANCE instance)
{
	if (!CreateWindowEx(
		0, WC_LISTVIEW, L"", WS_CHILD | WS_BORDER | LVS_REPORT,
		0, 0, 0, 0, main_window, (HMENU)ID_HWND_PROCESSES_LIST, instance, NULL))
	{
		return false;
	}

	if (!CreateWindowEx(
		0, WC_LISTVIEW, L"", WS_CHILD | WS_BORDER | LVS_REPORT,
		0, 0, 0, 0, main_window, (HMENU)ID_HWND_WINDOWS_LIST, instance, NULL))
	{
		return false;
	}

	if (!CreateWindowEx(
		0, WC_LISTVIEW, L"", WS_CHILD | WS_BORDER | LVS_REPORT,
		0, 0, 0, 0, main_window, (HMENU)ID_HWND_MESSAGES_LIST, instance, NULL))
	{
		return false;
	}

	if (!CreateWindowEx(
		0, MSFTEDIT_CLASS, L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY,
		0, 0, 0, 0, main_window, (HMENU)ID_HWND_INFO, instance, NULL))
	{
		return false;
	}

	create_list_columns(APP_MODE_PROCESSES, GetDlgItem(main_window, ID_HWND_PROCESSES_LIST));
	create_list_columns(APP_MODE_WINDOWS, GetDlgItem(main_window, ID_HWND_WINDOWS_LIST));
	create_list_columns(APP_MODE_MESSAGES, GetDlgItem(main_window, ID_HWND_MESSAGES_LIST));

	main_window_update_mode(main_window);

	return true;
}

static LRESULT CALLBACK main_window_proc(HWND hwnd, uint32_t msg, WPARAM wp, LPARAM lp)
{
	static app_data_t* app_data = NULL;
	static bool dragging_splitter = false;
	static int dragging_splitter_offset = 0;

	switch (msg)
	{
	case WM_CREATE:
	{
		const CREATESTRUCT* cs = (CREATESTRUCT*)lp;
		app_data = (app_data_t*)cs->lpCreateParams;

		if (!main_window_init(hwnd, cs->hInstance))
		{
			return -1;
		}
	} break;

	case WM_SIZE:
		if (wp != SIZE_MINIMIZED)
		{
			main_window_layout(hwnd);
		}
		break;

	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* mmi = (MINMAXINFO*)lp;
		mmi->ptMinTrackSize.x = to_pixels(320, GetDpiForWindow(hwnd));
		mmi->ptMinTrackSize.y = to_pixels(200, GetDpiForWindow(hwnd));
	} break;

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

		update_list_column_widths(GetDlgItem(hwnd, ID_HWND_PROCESSES_LIST));
		update_list_column_widths(GetDlgItem(hwnd, ID_HWND_WINDOWS_LIST));
		update_list_column_widths(GetDlgItem(hwnd, ID_HWND_MESSAGES_LIST));
	} break;

	case WM_SETCURSOR:
	{
		POINT pt;
		RECT client, splitter;
		if (dragging_splitter || get_layout_rects(hwnd, &client, &splitter) && GetCursorPos(&pt) && ScreenToClient(hwnd, &pt) && PtInRect(&splitter, pt))
		{
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			return TRUE;
		}
	}
	break;

	case WM_LBUTTONDOWN:
	{
		POINT pt;
		RECT client, splitter;
		if (get_layout_rects(hwnd, &client, &splitter) && GetCursorPos(&pt) && ScreenToClient(hwnd, &pt) && PtInRect(&splitter, pt))
		{
			dragging_splitter = true;
			dragging_splitter_offset = pt.x - splitter.left;
			SetCapture(hwnd);
		}
	}
	break;

	case WM_MOUSEMOVE:
		if (dragging_splitter)
		{
			POINT pt;
			RECT client;
			if (GetClientRect(hwnd, &client) && !IsRectEmpty(&client) && GetCursorPos(&pt) && ScreenToClient(hwnd, &pt))
			{
				int new_splitter_x = pt.x - dragging_splitter_offset;
				main_splitter_percent = (float)new_splitter_x / client.right;
				main_splitter_percent = max(0.1f, min(main_splitter_percent, 0.9f));
				main_window_layout(hwnd);
				//InvalidateRect(hwnd, NULL, FALSE);
			}
		}
		break;

	case WM_LBUTTONUP:
		if (dragging_splitter)
		{
			dragging_splitter = false;
			ReleaseCapture();
		}
		break;

	case WM_CAPTURECHANGED:
		dragging_splitter = false;
		break;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

bool init_ui(HINSTANCE instance, app_data_t* data)
{
	const INITCOMMONCONTROLSEX cc_init =
	{
		.dwSize = sizeof(INITCOMMONCONTROLSEX),
		.dwICC = 0xFFFF, // all common controls
	};

	const WNDCLASSEX wc_init =
	{
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = main_window_proc,
		.hInstance = instance,
		.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_MAIN)),
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1),
		.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU),
		.lpszClassName = L"DevSpyMainWindow",
		.hIconSm = LoadIcon(instance, MAKEINTRESOURCE(IDI_MAIN)),
	};

	if (!LoadLibrary(L"msftedit.dll") ||
		!InitCommonControlsEx(&cc_init) ||
		!RegisterClassEx(&wc_init))
	{
		return false;
	}

	HWND main_window = CreateWindowEx(
		0, wc_init.lpszClassName, L"", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, instance, data);

	if (!main_window)
	{
		return false;
	}

	main_window_layout(main_window);
	ShowWindow(main_window, SW_SHOWDEFAULT);

	return true;
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
