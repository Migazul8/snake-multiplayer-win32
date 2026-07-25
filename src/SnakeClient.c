#include "Snake.h"

WCHAR* wndClassName = L"SotoSnake_Window";
WCHAR* appName = L"Snake";

ATOM registerMyWindowClass();

HINSTANCE hInstance;
HANDLE hHeap;

static GameInstance gameInst;

bool isModernDPISupported = false;

pGetDpiForWindow GetDpiForWindowPtr = NULL;
pAdjustWindowRectExForDpi AdjustWindowRectExForDpiPtr = NULL;
pSystemParametersInfoForDpi SystemParametersInfoForDpiPtr = NULL;
pGetSystemMetricsForDpi GetSystemMetricsForDpiPtr = NULL;

static void loadDPIProcs();

void clientMain() {

    hInstance = GetModuleHandleW(NULL);
    hHeap = GetProcessHeap();

    loadDialogStrings(hInstance);

    loadDPIProcs();

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    if(DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_SERVERSELECT), NULL, ServerSelectDlgProc, &gameInst.serverConn) == IDOK) {
        MessageBoxW(NULL, L"ok", L"dd", MB_ICONINFORMATION);
    }

    WSACleanup();

    ExitProcess(0);

}

static void loadDPIProcs() {

    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    GetDpiForWindowPtr = (pGetDpiForWindow)GetProcAddress(hUser32, "GetDpiForWindow");
    AdjustWindowRectExForDpiPtr = (pAdjustWindowRectExForDpi)GetProcAddress(hUser32, "AdjustWindowRectExForDpi");
    SystemParametersInfoForDpiPtr = (pSystemParametersInfoForDpi)GetProcAddress(hUser32, "SystemParametersInfoForDpi");
    GetSystemMetricsForDpiPtr = (pGetSystemMetricsForDpi)GetProcAddress(hUser32, "GetSystemMetricsForDpi");

    isModernDPISupported = (GetDpiForWindowPtr && AdjustWindowRectExForDpiPtr && SystemParametersInfoForDpiPtr && GetSystemMetricsForDpiPtr);

}
