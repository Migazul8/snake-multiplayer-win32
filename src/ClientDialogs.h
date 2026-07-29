#define IDD_SERVERSELECT 1
#define IDD_SERVERCONNECT 2
#define IDD_LOBBYWAIT 3

#define WM_ENABLECONTROLS WM_APP+67
#define WM_ADDLANGAME WM_APP+20

typedef struct {

    HWND hWnd;

    SOCKET* socket;

}LanPeekThreadArgs;

typedef struct {

    SOCKADDR_IN addr;

    WCHAR portText[8];
    WCHAR ipText[32];

}LanGame;

typedef struct {

    ServerConnection* serverConn;

    HANDLE hLanSearchThread;
    SOCKET lanSearchSocket;

    LanPeekThreadArgs lanThreadArgs;

    size_t lanGameCount;
    LanGame lanGames[64];

}ServerSelectDlgData;

typedef struct {

    ServerConnection* serverConn;

    HANDLE hConnectThread;
    ServerConnectArgs args;

    bool showLoadCursor;

}ServerConnectDlgData;

INT_PTR CALLBACK LobbyWaitDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ServerSelectDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ServerConnectDlgProc(HWND, UINT, WPARAM, LPARAM);

void loadDialogStrings(HINSTANCE hInstance);
void loadUxthemeProcs();
void unloadUxtheme();
