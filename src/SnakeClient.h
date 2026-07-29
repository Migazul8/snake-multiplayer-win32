#define WM_SERVERCONNECTDONE WM_APP+10
#define WM_SERVERJOINDONE WM_APP+11

#define DEF_TILESIZE 10
#define DEF_GRIDW 40
#define DEF_GRIDH 25

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

    int32_t playerColor;

    WCHAR playerName[32];

}ServerConnectArgs;

void sendDirChange(ServerConnection* serverConn, char newDir);

void leaveServer(ServerConnection* serverConn);

DWORD WINAPI LanPeekerThreadEntry(void* hWnd);

DWORD WINAPI ServerConnectThreadEntry(ServerConnectArgs*);
DWORD WINAPI ServerJoinThreadEntry(ServerConnectArgs*);

typedef UINT (WINAPI* pGetDpiForWindow)(HWND hWnd);
typedef BOOL (WINAPI* pAdjustWindowRectExForDpi)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);
typedef BOOL (WINAPI* pSystemParametersInfoForDpi)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni, UINT dpi);
typedef int (WINAPI* pGetSystemMetricsForDpi)(int nIndex, UINT dpi);

typedef HRESULT (_stdcall* pSetWindowTheme)(HWND hWnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
