#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "psapi")
#pragma comment(lib, "shlwapi")

#include <windows.h>
#include <Shlwapi.h>
#include <psapi.h>

TCHAR szClassName[] = TEXT("Window");
HWND hList;
DWORD dwHangCount;

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	DWORD dwProcessID = 0;
	GetWindowThreadProcessId(hWnd, &dwProcessID);
	if (dwProcessID)
	{
		HANDLE hHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
		if (hHandle)
		{
			TCHAR szModuleFilePath[MAX_PATH];
			if (GetModuleFileNameEx(hHandle, NULL, szModuleFilePath, MAX_PATH))
			{
				SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)szModuleFilePath);
				if (StrStrI(PathFindFileName(szModuleFilePath), (LPCTSTR)lParam))
				{
					if (IsHungAppWindow(hWnd))
					{
						++dwHangCount;
						if (dwHangCount >= 5)
						{
							TerminateProcess(hHandle, 0);
							ShellExecute(NULL, TEXT("open"), szModuleFilePath, NULL, NULL, SW_SHOW);
						}
					}
					else
					{
						dwHangCount = 0;
					}
					CloseHandle(hHandle);
					return FALSE;
				}
			}
			CloseHandle(hHandle);
		}
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hButton1;
	static HWND hButton2;
	static HWND hEdit;
	switch (msg)
	{
	case WM_CREATE:
		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton1 = CreateWindow(TEXT("BUTTON"), TEXT("稼働"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)100, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton2 = CreateWindow(TEXT("BUTTON"), TEXT("停止"), WS_VISIBLE | WS_CHILD | WS_DISABLED, 0, 0, 0, 0, hWnd, (HMENU)101, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hList = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), 0, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hEdit, 10, 10, 256, 32, TRUE);
		MoveWindow(hButton1, 276, 10, 256, 32, TRUE);
		MoveWindow(hButton2, 542, 10, 256, 32, TRUE);
		MoveWindow(hList, 10, 50, LOWORD(lParam) - 20, HIWORD(lParam) - 60, 1);
		break;
	case WM_TIMER:
	{
		TCHAR szModuleFilePath[MAX_PATH];
		if (GetWindowText(hEdit, szModuleFilePath, MAX_PATH))
		{
			SendMessage(hList, LB_RESETCONTENT, 0, 0);
			EnumWindows(EnumWindowsProc, (LPARAM)szModuleFilePath);
		}
	}
	break;
	case WM_COMMAND:
		if (LOWORD(wParam) == 100)
		{
			if (GetWindowTextLength(hEdit) == 0)
			{
				MessageBox(hWnd, TEXT("ターゲットとなるEXEの名前を入力してください"), TEXT("未入力エラー"), 0);
				SetFocus(hEdit);
			}
			else
			{
				dwHangCount = 0;
				EnableWindow(hEdit, FALSE);
				EnableWindow(hButton1, FALSE);
				EnableWindow(hButton2, TRUE);
				SetTimer(hWnd, 0x1234, 1000 * 10, 0);
			}
		}
		else if (LOWORD(wParam) == 101)
		{
			KillTimer(hWnd, 0x1234);
			EnableWindow(hEdit, TRUE);
			EnableWindow(hButton1, TRUE);
			EnableWindow(hButton2, FALSE);
		}
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 0x1234);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("指定したEXE名が応答なしになっていた場合、強制終了して立ち上げなおす"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
