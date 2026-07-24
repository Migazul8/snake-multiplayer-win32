#include "Snake.h"

extern WCHAR* appName;

static WCHAR portText[16];
static WCHAR ipText[32];

extern HINSTANCE hInstance;
extern HANDLE hHeap;

//TODO: USE STRINGS FROM RESOURCES

INT_PTR CALLBACK ServerConnectDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    ServerConnectDlgData* dlgData = (ServerConnectDlgData*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

    switch(msg) {
        case WM_INITDIALOG:

            dlgData = (ServerConnectDlgData*)HeapAlloc(hHeap, 0, sizeof(ServerConnectDlgData));

            dlgData->serverConn = (ServerConnection*)lParam;

            SetWindowLongPtrW(hWnd, GWLP_USERDATA, dlgData);

            dlgData->args.hWnd = hWnd;
            dlgData->args.serverConn = dlgData->serverConn;

            return TRUE;

        case WM_SHOWWINDOW:
            if(wParam == TRUE && lParam == 0) {
                dlgData->hConnectThread = CreateThread(NULL, 0, ServerConnectThreadEntry, &dlgData->args, 0, NULL);
                return TRUE;
            }
            return FALSE;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDCANCEL:
                    //TODO: CANCEL CONNECTION
                    EndDialog(hWnd, IDCANCEL);
                    return TRUE;
            }
            break;

        case WM_SERVERCONNECTDONE: {
            ShowWindow(GetDlgItem(hWnd, 130), SW_HIDE);
            WCHAR buffer[512];
            switch(wParam) {
                case 0:

                    EnableWindow(GetDlgItem(hWnd, 101), TRUE);
                    EnableWindow(GetDlgItem(hWnd, 103), TRUE);
                    EnableWindow(GetDlgItem(hWnd, 201), TRUE);
                    EnableWindow(GetDlgItem(hWnd, 202), TRUE);
                    EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);
                    EnableWindow(GetDlgItem(hWnd, 102), TRUE);
                    EnableWindow(GetDlgItem(hWnd, 203), TRUE);
                    EnableWindow(GetDlgItem(hWnd, 204), TRUE);
                    EnableWindow(GetDlgItem(hWnd, 205), TRUE);
                    EnableWindow(GetDlgItem(hWnd, 105), TRUE);
                    EnableWindow(GetDlgItem(hWnd, 104), TRUE);

                    return TRUE;

                case 1:
                    swprintf_s(buffer, 512, L"Error %d while creating socket", (int)lParam);
                    MessageBoxW(hWnd, buffer, appName, MB_ICONERROR);
                    break;

                case 2:
                    swprintf_s(buffer, 512, L"Error %d while sending request", (int)lParam);
                    MessageBoxW(hWnd, buffer, appName, MB_ICONERROR);
                    break;

                case 3:
                    if(lParam == WSAETIMEDOUT) {
                        MessageBoxW(hWnd, L"Timeout while waiting for server response", appName, MB_ICONERROR);
                    }else{
                        switch(lParam) {
                            case 10054:
                                MessageBoxW(hWnd, L"The remote computer closed the connection.\nCheck the IP address and port introduced and try again later.", appName, MB_ICONERROR);
                                break;

                            case WSAEMSGSIZE:
                                MessageBoxW(hWnd, L"Invalid response", appName, MB_ICONERROR);
                                break;

                            default:
                                swprintf_s(buffer, 512, L"Error %d while receiving server response", (int)lParam);
                                MessageBoxW(hWnd, buffer, appName, MB_ICONERROR);
                                break;

                        }
                    }
                    break;

                case 4:
                    MessageBoxW(hWnd, L"Invalid server response", appName, MB_ICONERROR);
                    break;


            }

            EndDialog(hWnd, IDCANCEL);
            return TRUE;

        }

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
