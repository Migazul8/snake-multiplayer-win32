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

}GameInstance;

typedef struct {

    ServerConnection* serverConn;

    HWND hWnd;

}ServerConnectArgs;

DWORD WINAPI ServerConnectThreadEntry(ServerConnectArgs*);
