#define WM_SERVERCONNECTDONE WM_APP+10

typedef struct {

    SOCKADDR_IN addr;

    SOCKET clientSocket;

    int64_t secretNumber;

    uint64_t myPlayerID;

    struct {
        uint64_t playerCount;
        char serverName[64];
    }GameData;

}ServerConnection;

typedef struct{

    ServerConnection serverConn;

    HANDLE hGameThread;

    HWND hWnd;

    struct {
        UINT dpi;
        HFONT hPointsFont;
        HFONT hPlayerNamesFont;
        HBITMAP hBackbuffer;
        HDC hBackbufferDC;
        int tileSize;
    }windowData;

    int foodX;
    int foodY;

    uint64_t playerListLenght;

}GameInstance;

typedef struct {

    ServerConnection* serverConn;

    HWND hWnd;

}ServerConnectArgs;

DWORD WINAPI ServerConnectThreadEntry(ServerConnectArgs*);

typedef UINT (WINAPI* pGetDpiForWindow)(HWND hWnd);
typedef BOOL (WINAPI* pAdjustWindowRectExForDpi)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);
typedef BOOL (WINAPI* pSystemParametersInfoForDpi)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni, UINT dpi);
typedef int (WINAPI* pGetSystemMetricsForDpi)(int nIndex, UINT dpi);
