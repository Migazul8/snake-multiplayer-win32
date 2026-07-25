typedef struct {

    SOCKADDR_IN inAddr;

    int64_t secretNumber;

}LobbyPlayerConnection;

typedef struct {

    uint32_t maxConns;
    LobbyPlayerConnection* playerConns;

}LobbyServerData;

typedef struct {

    uint64_t points;

    uint32_t bodyLenght;
    struct {
        int32_t x;
        int32_t y;
    }bodyParts;

}PlayerData;

typedef struct {

    SOCKET serverSocket;
    HANDLE hListenerThread;

    int32_t gridW;
    int32_t gridH;

    int32_t foodX;
    int32_t foodY;

    uint32_t maxPlayers;
    PlayerData* playerData;

    bool hasGameStarted;

    LobbyServerData* lobbyData;

    char serverName[64];
    WCHAR wServerName[64];

}ServerInstance;

void closeServer(ServerInstance* serverInst);

int createServer(ServerInstance* serverInst, WCHAR* port, bool lanVisibility,
                 WCHAR* serverName, uint32_t maxPlayers, int32_t gridW, int32_t gridH,
                 bool growing, uint32_t initialLenght, int* wsaError);

DWORD WINAPI PacketListenerThreadEntry(ServerInstance*);
