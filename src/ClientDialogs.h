#define IDD_SERVERSELECT 1
#define IDD_SERVERCONNECT 2

typedef struct {

    SOCKADDR_IN addr;

    WCHAR portText[6];
    WCHAR ipText[32];

}LanGame;

typedef struct {

    ServerConnection* serverConn;

    size_t lanGameCount;
    LanGame lanGames[64];

}ServerSelectDlgData;

typedef struct {

    ServerConnection* serverConn;

    HWND hWnd;

}ConnectArgs;

typedef struct {

    ServerConnection* serverConn;

    HANDLE hConnectThread;
    ConnectArgs args;

}ServerConnectDlgData;

INT_PTR CALLBACK ServerSelectDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ServerConnectDlgProc(HWND, UINT, WPARAM, LPARAM);

void loadStrings(HINSTANCE hInstance);
