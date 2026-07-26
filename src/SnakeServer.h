typedef struct {

    struct {
        int64_t secretNumber;

        SOCKADDR_IN addr;

        bool slotFree;

        bool online;

    }connection;

    uint64_t points;

    uint32_t bodyLenght;
    struct {
        int32_t x;
        int32_t y;
    } bodyParts[1024];

    bool active;

}PlayerData;

typedef struct {

    SOCKET serverSocket;
    HANDLE hListenerThread;

    int32_t gridW;
    int32_t gridH;

    int32_t foodX;
    int32_t foodY;

    int32_t foodType;

    uint32_t currentPlayerCount;

    uint32_t maxPlayers;
    uint32_t playerDataLenght;
    PlayerData* playerData;

    bool hasGameStarted;

    char serverName[64];
    WCHAR wServerName[64];

}ServerInstance;

void closeServer(ServerInstance* serverInst);

int createServer(ServerInstance* serverInst, WCHAR* port, bool lanVisibility,
                 WCHAR* serverName, uint32_t maxPlayers, int32_t gridW, int32_t gridH,
                 bool growing, uint32_t initialLenght, int* wsaError);

DWORD WINAPI PacketListenerThreadEntry(ServerInstance*);
