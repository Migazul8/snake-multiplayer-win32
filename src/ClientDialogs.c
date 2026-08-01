#include "Snake.h"

#include "winerror.h"

extern WCHAR* appName;

static WCHAR portText[16];
static WCHAR ipText[32];

static WCHAR connClosedText[256];
static WCHAR timedOutText[128];
static WCHAR invalidResponseText[128];
static WCHAR invalidPortText[128];

static INT_PTR handleSysViewNotify(NMHDR* hdr, ServerSelectDlgData* dlgData, HWND hWnd);

extern HINSTANCE hInstance;
extern HANDLE hHeap;

extern bool isModernDPISupported;

extern pGetDpiForWindow GetDpiForWindowPtr;
extern pAdjustWindowRectExForDpi AdjustWindowRectExForDpiPtr;
extern pSystemParametersInfoForDpi SystemParametersInfoForDpiPtr;
extern pGetSystemMetricsForDpi GetSystemMetricsForDpiPtr;

static HCURSOR hLoadCursor;

static HMODULE hUxtheme = NULL;
static pSetWindowTheme SetWindowThemePtr = NULL;

INT_PTR CALLBACK LobbyWaitDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    ServerConnection* serverConn = (ServerConnection*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

    switch(msg) {
        case WM_INITDIALOG:
            serverConn = (ServerConnection*)lParam;
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, serverConn);

            return TRUE;


        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDCLOSE:
                    leaveServer(serverConn);
                    EndDialog(hWnd, IDCLOSE);
                    return TRUE;
            }

            break;

        case WM_CLOSE:
            leaveServer(serverConn);
            EndDialog(hWnd, IDCLOSE);
            return TRUE;

    }
    return FALSE;

}

INT_PTR CALLBACK ServerConnectDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    ServerConnectDlgData* dlgData = (ServerConnectDlgData*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

    switch(msg) {
        case WM_INITDIALOG:

            dlgData = (ServerConnectDlgData*)HeapAlloc(hHeap, 0, sizeof(ServerConnectDlgData));

            dlgData->serverConn = (ServerConnection*)lParam;

            SetWindowLongPtrW(hWnd, GWLP_USERDATA, dlgData);

            dlgData->args.hWnd = hWnd;
            dlgData->args.serverConn = dlgData->serverConn;

            if(hLoadCursor == NULL) {
                hLoadCursor = LoadCursorW(NULL, IDC_APPSTARTING);
            }

            dlgData->showLoadCursor = true;

            return TRUE;

        case WM_SHOWWINDOW:
            if(wParam == TRUE && lParam == 0) {
                dlgData->hConnectThread = CreateThread(NULL, 0, ServerConnectThreadEntry, &dlgData->args, 0, NULL);
                return TRUE;
            }
            return FALSE;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDOK:

                    dlgData->showLoadCursor = true;

                    GetDlgItemTextW(hWnd, 102, dlgData->args.playerName, 32);
                    dlgData->args.playerColor = SendDlgItemMessageW(hWnd, 105, CB_GETCURSEL, 0, 0);

                    dlgData->hConnectThread = CreateThread(NULL, 0, ServerJoinThreadEntry, &dlgData->args, 0, NULL);

                    ShowWindow(GetDlgItem(hWnd, 130), SW_SHOW);
                    SendMessageW(hWnd, WM_ENABLECONTROLS, 0, FALSE);

                    return TRUE;

                case IDCANCEL:
                    TerminateThread(dlgData->hConnectThread, 0);
                    closesocket(dlgData->serverConn->clientSocket);
                    EndDialog(hWnd, IDCANCEL);
                    return TRUE;
            }
            break;

        case WM_SERVERJOINDONE: {
            dlgData->showLoadCursor = false;
            ShowWindow(GetDlgItem(hWnd, 130), SW_HIDE);
            SendMessageW(hWnd, WM_ENABLECONTROLS, 0, TRUE);
            WCHAR buffer[512];
            switch(wParam) {
                case 0:
                    switch(lParam) {
                        case 0:
                            EndDialog(hWnd, IDOK);
                            break;

                        case 1:
                            MessageBoxW(hWnd, L"Game already started", appName, MB_ICONERROR);
                            EndDialog(hWnd, IDCANCEL);
                            break;

                        case 2:
                            MessageBoxW(hWnd, L"Lobby full of lil bros", appName, MB_ICONERROR);
                            break;

                        default:
                            MessageBoxW(hWnd, invalidResponseText, appName, MB_ICONERROR);
                            break;
                    }
                    break;

                case 1:
                    MessageBoxW(hWnd, L"error creatin socke", appName, MB_ICONERROR);
                    break;

                case 2:
                    MessageBoxW(hWnd, L"error sendin sock", appName, MB_ICONERROR);
                    break;

                case 3:
                    switch(lParam) {
                        case WSAETIMEDOUT:
                            MessageBoxW(hWnd, timedOutText, appName, MB_ICONERROR);
                            EndDialog(hWnd, IDCANCEL);
                            break;

                        case 10054:
                            MessageBoxW(hWnd, connClosedText, appName, MB_ICONERROR);
                            EndDialog(hWnd, IDCANCEL);
                            break;

                        default:

                            swprintf_s(buffer, 512, L"Error %d while recieving data", lParam);
                            MessageBoxW(hWnd, buffer, appName, MB_ICONERROR);
                            break;

                    }
                    break;

                case 4:
                    MessageBoxW(hWnd, invalidResponseText, appName, MB_ICONERROR);
                    break;

            }

        }
            return TRUE;


        case WM_SERVERCONNECTDONE: {
            dlgData->showLoadCursor = false;
            ShowWindow(GetDlgItem(hWnd, 130), SW_HIDE);
            WCHAR buffer[512];
            switch(wParam) {
                case 0:

                    ServerInfoPacket* sip = (ServerInfoPacket*)lParam;

                    MultiByteToWideChar(CP_UTF8, 0, sip->serverName, 64, buffer, 512);

                    SetDlgItemTextW(hWnd, 101, buffer);

                    swprintf_s(buffer, 512, L"%u/%u", sip->currentPlayerCount, sip->maxPlayers);
                    SetDlgItemTextW(hWnd, 103, buffer);

                    swprintf_s(buffer, 512, L"%u", sip->playersGrow);
                    SetDlgItemTextW(hWnd, 104, buffer);

                    swprintf_s(buffer, 512, L"%ux%u", sip->gridW, sip->gridH);
                    SetDlgItemTextW(hWnd, 106, buffer);

                    swprintf_s(buffer, 512, L"%u", sip->initialLenght);
                    SetDlgItemTextW(hWnd, 107, buffer);

                    SendMessageW(hWnd, WM_ENABLECONTROLS, 0, TRUE);

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
                    switch(lParam) {
                        case WSAETIMEDOUT:
                            MessageBoxW(hWnd, timedOutText, appName, MB_ICONERROR);
                            break;

                        case 10054:
                            MessageBoxW(hWnd, connClosedText, appName, MB_ICONERROR);
                            break;

                        case WSAEMSGSIZE:
                            MessageBoxW(hWnd, invalidResponseText, appName, MB_ICONERROR);
                            break;

                        default:
                            swprintf_s(buffer, 512, L"Error %d while receiving server response", (int)lParam);
                            MessageBoxW(hWnd, buffer, appName, MB_ICONERROR);
                            break;

                    }
                    break;

                case 4:
                    MessageBoxW(hWnd, invalidResponseText, appName, MB_ICONERROR);
                    break;


            }

            EndDialog(hWnd, IDCANCEL);
            return TRUE;

        }

        case WM_ENABLECONTROLS:
            EnableWindow(GetDlgItem(hWnd, 101), lParam);
            EnableWindow(GetDlgItem(hWnd, 103), lParam);
            EnableWindow(GetDlgItem(hWnd, 201), lParam);
            EnableWindow(GetDlgItem(hWnd, 202), lParam);
            EnableWindow(GetDlgItem(hWnd, IDOK), lParam);
            EnableWindow(GetDlgItem(hWnd, 102), lParam);
            EnableWindow(GetDlgItem(hWnd, 203), lParam);
            EnableWindow(GetDlgItem(hWnd, 204), lParam);
            EnableWindow(GetDlgItem(hWnd, 205), lParam);
            EnableWindow(GetDlgItem(hWnd, 105), lParam);
            EnableWindow(GetDlgItem(hWnd, 104), lParam);
            EnableWindow(GetDlgItem(hWnd, 206), lParam);
            EnableWindow(GetDlgItem(hWnd, 106), lParam);
            EnableWindow(GetDlgItem(hWnd, 207), lParam);
            EnableWindow(GetDlgItem(hWnd, 107), lParam);
            return TRUE;

        case WM_SETCURSOR:
            if(dlgData->showLoadCursor) {
                SetCursor(hLoadCursor);
                return TRUE;
            }
            break;

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

            dlgData->lanThreadArgs.hWnd = hWnd;
            dlgData->lanThreadArgs.socket = &dlgData->lanSearchSocket;

            SetWindowLongPtrW(hWnd, GWLP_USERDATA, dlgData);

            SendDlgItemMessageW(hWnd, 111, IPM_SETADDRESS, 0, MAKEIPADDRESS(127, 0, 0, 1));

            SendDlgItemMessageW(hWnd, 112, EM_SETCUEBANNER, TRUE, (LPARAM)L"29349");

            SendDlgItemMessageW(hWnd, 110, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);

            UINT dpi;

            if(isModernDPISupported) {
                dpi = GetDpiForWindowPtr(hWnd);
            }else{
                HDC hScreen = GetDC(NULL);
                dpi = GetDeviceCaps(hScreen, LOGPIXELSX);
                ReleaseDC(NULL, hScreen);
            }

            LVCOLUMNW col = {};
                col.mask = LVCF_TEXT | LVCF_WIDTH;
                col.pszText = ipText;
                col.cx = MulDiv(110, dpi, USER_DEFAULT_SCREEN_DPI);;
            SendDlgItemMessageW(hWnd, 110, LVM_INSERTCOLUMNW, 0, (LPARAM)&col);

                col.mask = LVCF_TEXT | LVCF_WIDTH;
                col.pszText = portText;
                col.cx = MulDiv(55, dpi, USER_DEFAULT_SCREEN_DPI);
            SendDlgItemMessageW(hWnd, 110, LVM_INSERTCOLUMNW, 1, (LPARAM)&col);

            if(hUxtheme) {
                SetWindowTheme(GetDlgItem(hWnd, 110), L"Explorer", NULL);
            }

            dlgData->lanSearchSocket = INVALID_SOCKET;
            dlgData->hLanSearchThread = NULL;

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
                        MessageBoxW(hWnd, invalidPortText, appName, MB_ICONERROR);
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


        case WM_TIMER:
            if(wParam == 10) {

                if(dlgData->lanSearchSocket == INVALID_SOCKET) {
                    TerminateThread(dlgData->hLanSearchThread, 0);
                    closesocket(dlgData->lanSearchSocket);
                    dlgData->lanGameCount = 0;
                    SendDlgItemMessageW(hWnd, 110, LVM_SETITEMCOUNT, dlgData->lanGameCount, 0);
                    dlgData->hLanSearchThread = CreateThread(NULL, 0, LanPeekerThreadEntry, &dlgData->lanThreadArgs, 0, NULL);
                }



                return TRUE;
            }
            break;

        case WM_ADDLANGAME: {

            if(dlgData->lanGameCount >= 32) return TRUE;

            LanGame* lanGame = (LanGame*)lParam;

            dlgData->lanGames[dlgData->lanGameCount].addr = lanGame->addr;

            swprintf_s(dlgData->lanGames[dlgData->lanGameCount].ipText, 32, L"%u.%u.%u.%u", lanGame->addr.sin_addr.s_net,
                                                   lanGame->addr.sin_addr.s_host,
                                                   lanGame->addr.sin_addr.s_lh,
                                                   lanGame->addr.sin_addr.s_impno);

            swprintf_s(dlgData->lanGames[dlgData->lanGameCount].portText, 8, L"%u", ntohs(lanGame->addr.sin_port));

            dlgData->lanGameCount++;
            SendDlgItemMessageW(hWnd, 110, LVM_SETITEMCOUNT, dlgData->lanGameCount, 0);


        }
            return TRUE;

        case WM_NOTIFY: {
            if(((NMHDR*)lParam)->idFrom == 110) {
                return handleSysViewNotify((NMHDR*)lParam, dlgData, hWnd);
            }
        }

            break;

        case WM_DESTROY:

            if(dlgData->lanSearchSocket != INVALID_SOCKET) {
                TerminateThread(dlgData->hLanSearchThread, 0);
                closesocket(dlgData->lanSearchSocket);
            }

            HeapFree(hHeap, 0, dlgData);
            KillTimer(hWnd, 10);
            return TRUE;

    }
    return FALSE;

}

static INT_PTR handleSysViewNotify(NMHDR* hdr, ServerSelectDlgData* dlgData, HWND hWnd) {
    NMITEMACTIVATE* ia = (NMITEMACTIVATE*)hdr;
    NMLVDISPINFOW* dispInfo = (NMLVDISPINFOW*)hdr;
    switch(hdr->code) {
        case LVN_GETDISPINFOW:
            NMLVDISPINFOW* dispInfo = (NMLVDISPINFOW*)hdr;
            switch(dispInfo->item.iSubItem) {
                case 0:
                    dispInfo->item.pszText = dlgData->lanGames[dispInfo->item.iItem].ipText;

                    return TRUE;

                case 1:
                    dispInfo->item.pszText = dlgData->lanGames[dispInfo->item.iItem].portText;
                    return TRUE;
            }

            break;

        case NM_CLICK:
            if(ia->iItem < 0) break;
        case LVN_ITEMACTIVATE:
            if(ia->iItem >= dlgData->lanGameCount) break;

            dlgData->serverConn->addr = dlgData->lanGames[ia->iItem].addr;

            if(DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_SERVERCONNECT), hWnd, ServerConnectDlgProc, dlgData->serverConn) == IDOK) {
                EndDialog(hWnd, IDOK);
            }

            return TRUE;
    }

    return FALSE;
}

void loadDialogStrings(HINSTANCE hInstance) {

    LoadStringW(hInstance, 1, portText, 16);
    LoadStringW(hInstance, 2, ipText, 32);
    LoadStringW(hInstance, 3, connClosedText, 256);
    LoadStringW(hInstance, 4, timedOutText, 128);
    LoadStringW(hInstance, 5, invalidResponseText, 128);
    LoadStringW(hInstance, 6, invalidPortText, 128);

}

void loadUxthemeProcs() {

    hUxtheme = LoadLibraryW(L"uxtheme.dll");
    if(hUxtheme) {
        SetWindowThemePtr = (pSetWindowTheme)GetProcAddress(hUxtheme, "SetWindowTheme");
    }
}

void unloadUxtheme() {

    if(hUxtheme) {
        FreeLibrary(hUxtheme);
    }

}
