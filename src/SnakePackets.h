#define GP_SERVERINFO 1
#define GP_JOINREQUEST 2
#define GP_JOINANSWER 3
#define GP_LANPEEK 4
#define GP_LEAVESERVER 5
#define GP_CLOSESERVER 6
#define GP_STARTGAME 7
#define GP_CHANGEPLAYERDIR 8
#define GP_GAMEUPDATE 9
#define GP_PLAYERNAME 10
#define GP_PLAYERCOUNT 11

typedef struct {

    char magic[2];

    int32_t type;

}GamePacket;

typedef struct {

    GamePacket gp;

    uint64_t maxPlayers;
    uint64_t currentPlayerCount;

    char serverName[64];

}ServerInfoPacket;

typedef struct {

    GamePacket gp;

    char playerName[32];

    int32_t playerColor;

}JoinRequestPacket;

typedef struct {

    GamePacket gp;

    int32_t returnCode;

    int64_t secretNumber;

    uint64_t playerID;

}JoinAnswerPacket;

typedef struct {

    GamePacket gp;

    int64_t secretNumber;

    uint64_t playerID;

}LeavePacket;

typedef struct {

    GamePacket gp;

    uint64_t playerID;

    int64_t secretNumber;

    uint8_t newDir;

}DirChangePacket;

typedef struct {

    GamePacket gp;

    USHORT port;

}LanPeekAnswer;

void initPacket(GamePacket* gp, int32_t packetType);
bool isPacketValid(GamePacket* gp);
