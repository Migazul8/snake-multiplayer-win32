#include "Snake.h"

extern WCHAR* appName;

static WCHAR portText[16];
static WCHAR ipText[32];

extern HINSTANCE hInstance;
extern HANDLE hHeap;

INT_PTR CALLBACK ServerConnectDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    ServerConnectDlgData* dlgData = (ServerConnectDlgData*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

    switch(msg) {
        case WM_INITDIALOG:

            dlgData = (ServerConnectDlgData*)HeapAlloc(hHeap, 0, sizeof(ServerConnectDlgData));

            dlgData->serverConn = (ServerConnection*)lParam;

            SetWindowLongPtrW(hWnd, GWLP_USERDATA, dlgData);

            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDCANCEL:
                    EndDialog(hWnd, IDCANCEL);
                    return TRUE;
            }
            break;

        case WM_SERVERCONNECTDONE:
            switch(wParam) {
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    break;


            }
            return TRUE;

        case WM_DESTROY:
            HeapFree(hHeap, 0, dlgData);

            return TRUE;
    }
    return FALSE;

}

INT_PTR CALLBACK ServerSelectDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    ServerSelectDlgData* dlgData = (ServerSelectDlgData*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

    switch(msg) {
        case WM_INITDIALOG:

            dlgData = (ServerSelectDlgData*)HeapAlloc(hHeap, 0, sizeof(ServerSelectDlgData));
            dlgData->lanGameCount = 0;
            dlgData->serverConn = (ServerConnection*)lParam;

            SetWindowLongPtrW(hWnd, GWLP_USERDATA, dlgData);

            SendDlgItemMessageW(hWnd, 111, IPM_SETADDRESS, 0, MAKEIPADDRESS(127, 0, 0, 1));

            SendDlgItemMessageW(hWnd, 112, EM_SETCUEBANNER, TRUE, (LPARAM)L"29349");

            LVCOLUMNW col = {};
                col.mask = LVCF_TEXT | LVCF_WIDTH;
                col.pszText = ipText;
                col.cx = 200;
            SendDlgItemMessageW(hWnd, 110, LVM_INSERTCOLUMNW, 0, (LPARAM)&col);

                col.mask = LVCF_TEXT | LVCF_WIDTH;
                col.pszText = portText;
                col.cx = 100;
            SendDlgItemMessageW(hWnd, 110, LVM_INSERTCOLUMNW, 1, (LPARAM)&col);

            SetTimer(hWnd, 10, 3000, NULL);
            SendMessageW(hWnd, WM_TIMER, 10, NULL);

            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case 120:

                    WCHAR portBuffer[8];
                    GetDlgItemTextW(hWnd, 112, portBuffer, 8);
                    int iPort = _wtoi(portBuffer);
                    if(iPort < 1024 || iPort > 49151) {
                        MessageBoxW(hWnd, L"invalid port", appName, MB_ICONERROR);
                        return FALSE;
                    }

                    dlgData->serverConn->addr.sin_port = htons((USHORT)iPort);

                    DWORD ip;
                    SendDlgItemMessageW(hWnd, 111, IPM_GETADDRESS, 0, &ip);
                    dlgData->serverConn->addr.sin_addr.s_addr = htonl(ip);
                    dlgData->serverConn->addr.sin_family = AF_INET;

                    if(DialogBoxParamW(hInstance, IDD_SERVERCONNECT, hWnd, ServerConnectDlgProc, dlgData->serverConn) == IDOK) {
                        EndDialog(hWnd, IDOK);
                    }

                    return TRUE;

                case IDCANCEL:
                    EndDialog(hWnd, IDCANCEL);
                    return TRUE;
            }
            break;

        case WM_DESTROY:
            HeapFree(hHeap, 0, dlgData);
            KillTimer(hWnd, 10);
            return TRUE;

    }
    return FALSE;

}

void loadStrings(HINSTANCE hInstance) {

    LoadStringW(hInstance, 1, portText, 16);
    LoadStringW(hInstance, 2, ipText, 32);

}
