#include "Snake.h"

INT_PTR ServerCreateDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_INITDIALOG:

            SendDlgItemMessageW(hWnd, 102, UDM_SETRANGE32, 1, 999);
            SendDlgItemMessageW(hWnd, 102, UDM_SETPOS32, 0, 5);

            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDCANCEL:
                    EndDialog(hWnd, IDCANCEL);
                    return TRUE;
            }

            break;
    }
    return FALSE;

}
