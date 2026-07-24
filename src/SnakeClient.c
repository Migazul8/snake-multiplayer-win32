#include "Snake.h"

WCHAR* wndClassName = L"SotoSnake_Window";
WCHAR* appName = L"Snake";

ATOM registerMyWindowClass();

HINSTANCE hInstance;
HANDLE hHeap;

static GameInstance gameInst;

void clientMain() {

    hInstance = GetModuleHandleW(NULL);
    hHeap = GetProcessHeap();

    loadStrings(hInstance);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    if(DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_SERVERSELECT), NULL, ServerSelectDlgProc, &gameInst.serverConn) == IDOK) {
        MessageBoxW(NULL, L"ok", L"dd", MB_ICONINFORMATION);
    }

    WSACleanup();

    ExitProcess(0);

}
