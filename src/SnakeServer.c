#include "Snake.h"

HINSTANCE hInstance;
HANDLE hHeap;

void serverMain() {

    hInstance = GetModuleHandleW(NULL);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    DialogBoxParamW(hInstance, IDD_CREATESERVER, NULL, ServerCreateDlgProc, 0);

    WSACleanup();

    ExitProcess(0);

}
