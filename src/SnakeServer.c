#include "Snake.h"

HINSTANCE hInstance;
static HANDLE hHeap;

static ServerInstance serverInst = {};

void serverMain() {

    hInstance = GetModuleHandleW(NULL);

    loadDialogsStrings(hInstance);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    if(DialogBoxParamW(hInstance, IDD_CREATESERVER, NULL, ServerCreateDlgProc, &serverInst) == IDOK) {
        serverInst.hListenerThread = CreateThread(NULL, 0, PacketListenerThreadEntry, &serverInst, 0, NULL);
        DialogBoxParamW(hInstance, IDD_SERVERPANEL, NULL, ServerPanelDlgProc, &serverInst);
    }

    WSACleanup();

    ExitProcess(0);

}
