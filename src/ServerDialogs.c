#include "Snake.h"

WCHAR* appName = L"Snake Server";

static WCHAR maxPlayersWarningText[128];
static WCHAR portInUseText[128];
static WCHAR invalidPortText[128];

INT_PTR ServerPanelDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    ServerInstance* serverInst = (ServerInstance*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

    switch(msg) {
        case WM_INITDIALOG:
            serverInst = (ServerInstance*)lParam;
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, serverInst);

            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case 102:
                    closeServer(serverInst);
                    EndDialog(hWnd, IDCLOSE);
                    return TRUE;

            }
            break;

        case WM_CLOSE:
            closeServer(serverInst);
            EndDialog(hWnd, IDCLOSE);
            return TRUE;

    }
    return FALSE;

}

INT_PTR ServerCreateDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    ServerInstance* serverInst = (ServerInstance*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

    switch(msg) {
        case WM_INITDIALOG:

            serverInst = (ServerInstance*)lParam;
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, serverInst);

            SendDlgItemMessageW(hWnd, 101, EM_SETCUEBANNER, TRUE, L"29349");
            SendDlgItemMessageW(hWnd, 103, EM_SETCUEBANNER, TRUE, L"My sussy server...");

            SendDlgItemMessageW(hWnd, 102, UDM_SETRANGE32, 1, 999);
            SendDlgItemMessageW(hWnd, 102, UDM_SETPOS32, 0, 5);

            SendDlgItemMessageW(hWnd, 106, UDM_SETRANGE32, 10, 100);
            SendDlgItemMessageW(hWnd, 106, UDM_SETPOS32, 0, 40);

            SendDlgItemMessageW(hWnd, 107, UDM_SETRANGE32, 10, 100);
            SendDlgItemMessageW(hWnd, 107, UDM_SETPOS32, 0, 25);

            SendDlgItemMessageW(hWnd, 105, BM_SETCHECK, BST_CHECKED, 0);

            SendDlgItemMessageW(hWnd, 104, BM_SETCHECK, BST_CHECKED, 0);

            SendDlgItemMessageW(hWnd, 108, UDM_SETRANGE32, 1, 5);
            SendDlgItemMessageW(hWnd, 108, UDM_SETPOS32, 0, 2);

            SendDlgItemMessageW(hWnd, 502, UDM_SETRANGE32, 1, 60);
            SendDlgItemMessageW(hWnd, 502, UDM_SETPOS32, 0, 10);

            ShowWindow(GetDlgItem(hWnd, 600), SW_HIDE);
            ShowWindow(GetDlgItem(hWnd, 601), SW_HIDE);
            ShowWindow(GetDlgItem(hWnd, 501), SW_HIDE);
            ShowWindow(GetDlgItem(hWnd, 502), SW_HIDE);

            return TRUE;

        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case IDOK:

                    WCHAR port[6];
                    GetDlgItemTextW(hWnd, 101, port, 6);
                    int iPort = _wtoi(port);
                    if(iPort < 1024 || iPort > 49151) {
                        MessageBoxW(hWnd, invalidPortText, appName, MB_ICONERROR);
                        return TRUE;
                    }

                    bool lanVisibility = (SendDlgItemMessageW(hWnd, 104, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    if(lanVisibility && iPort == 29350) {
                        if(MessageBoxW(hWnd, L"//TODO: lanWarnText", appName, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDNO) {
                            return TRUE;
                        }
                    }

                    WCHAR serverName[64];
                    GetDlgItemTextW(hWnd, 103, serverName, 64);

                    uint32_t maxPlayers = abs(SendDlgItemMessageW(hWnd, 102, UDM_GETPOS32, 0, 0));
                    if(maxPlayers >= 8) {
                        if(MessageBoxW(hWnd, maxPlayersWarningText, appName, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1) == IDNO) {
                            return TRUE;
                        }
                    }

                    int32_t gridW = SendDlgItemMessageW(hWnd, 106, UDM_GETPOS32, 0, 0);
                    int32_t gridH = SendDlgItemMessageW(hWnd, 107, UDM_GETPOS32, 0, 0);

                    bool playerGrowing = (SendDlgItemMessageW(hWnd, 105, BM_GETCHECK, 0, 0) == BST_CHECKED);

                    uint32_t initialPlayerLen = abs(SendDlgItemMessageW(hWnd, 108, UDM_GETPOS32, 0, 0));


                    int wsaErr = 0;
                    int ret = createServer(serverInst, port, lanVisibility, serverName, maxPlayers, gridW, gridH, playerGrowing, initialPlayerLen, &wsaErr);
                    WCHAR buffer[512];
                    switch(ret) {
                        case 3:
                            MessageBoxW(NULL, L"Couldn't bind AF_INET socket, program could crash at any moment", appName, MB_ICONWARNING);
                        case 0:
                            EndDialog(hWnd, IDOK);
                            break;

                        case 1:
                            swprintf_s(buffer, 512, L"error %d when creating socket", wsaErr);
                            MessageBoxW(hWnd, buffer, appName, MB_ICONERROR);
                            break;

                        case 2:
                            switch(wsaErr) {
                                case WSAEADDRINUSE:
                                    MessageBoxW(hWnd, portInUseText, appName, MB_ICONERROR);
                                    break;

                                default:
                                    swprintf_s(buffer, 512, L"error %d when binding socket", wsaErr);
                                    MessageBoxW(hWnd, buffer, appName, MB_ICONERROR);
                                    break;
                            }
                            break;

                    }

                    return TRUE;
                case IDCANCEL:
                    EndDialog(hWnd, IDCANCEL);
                    return TRUE;
            }

        }
            break;

        case WM_RBUTTONDBLCLK:
            if(wParam & MK_CONTROL) {
                ShowWindow(GetDlgItem(hWnd, 600), SW_SHOW);
                ShowWindow(GetDlgItem(hWnd, 601), SW_SHOW);
                ShowWindow(GetDlgItem(hWnd, 501), SW_SHOW);
                ShowWindow(GetDlgItem(hWnd, 502), SW_SHOW);
            }
            return TRUE;
    }
    return FALSE;

}

void loadDialogsStrings(HINSTANCE hInstance) {

    LoadStringW(hInstance, 1, maxPlayersWarningText, 128);
    LoadStringW(hInstance, 2, portInUseText, 128);
    LoadStringW(hInstance, 3, invalidPortText, 128);

}
