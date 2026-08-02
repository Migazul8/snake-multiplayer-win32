#include "Snake.h"

static HANDLE hHeap;

static void broadcastPacket(GamePacket* gp, int packetSize, ServerInstance* serverInst);

static void handlePacket(GamePacket* gp, SOCKADDR_IN* fromAddr, ServerInstance* serverInst);

static void addPlayer(JoinRequestPacket* jr, ServerInstance* serverInst, SOCKADDR_IN* addr, JoinAnswerPacket* ja);
static void removePlayer(LeavePacket* lp, ServerInstance* serverInst);

static DWORD WINAPI LanListenerThreadEntry(LanBroadcastingData*);

void handlePacket(GamePacket* gp, SOCKADDR_IN* fromAddr, ServerInstance* serverInst) {
    switch(gp->type) {
        case GP_SERVERINFO:
            if(serverInst->hasGameStarted) return;

            ServerInfoPacket sip;
            initPacket(&sip.gp, GP_SERVERINFO);
            sip.currentPlayerCount = serverInst->currentPlayerCount;
            sip.gridW = serverInst->gridW;
            sip.gridH = serverInst->gridH;
            sip.maxPlayers = serverInst->maxPlayers;
            sip.playersGrow = (uint8_t)serverInst->playersGrow;
            sip.initialLenght = serverInst->initialPlayerLenght;
            strcpy_s(sip.serverName, 64, serverInst->serverName);

            sendto(serverInst->serverSocket, &sip, sizeof(ServerInfoPacket), 0, fromAddr, sizeof(SOCKADDR_IN));

            break;

        case GP_JOINREQUEST:
            JoinAnswerPacket ja;
            addPlayer((JoinRequestPacket*)gp, serverInst, fromAddr, &ja);

            sendto(serverInst->serverSocket, &ja, sizeof(JoinAnswerPacket), 0, fromAddr, sizeof(SOCKADDR_IN));

            break;

        case GP_LEAVESERVER:
            removePlayer((LeavePacket*)gp, serverInst);

            break;
    }


}

DWORD WINAPI PacketListenerThreadEntry(ServerInstance* serverInst) {

    SOCKADDR_IN fromAddr;
    int fromAddrSize = sizeof(SOCKADDR_IN);

    char buffer[512];
    while(true) {
        recvfrom(serverInst->serverSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&fromAddr, &fromAddrSize);
        if(isPacketValid((GamePacket*)buffer)) {
            handlePacket((GamePacket*)buffer, &fromAddr, serverInst);
        }
    }

    return 0;

}

void addPlayer(JoinRequestPacket* jr, ServerInstance* serverInst, SOCKADDR_IN* addr, JoinAnswerPacket* ja) {

    initPacket(&ja->gp, GP_JOINANSWER);

    if(serverInst->hasGameStarted) {
        ja->returnCode = 1;
        return;
    }

    if(serverInst->currentPlayerCount >= serverInst->maxPlayers) {
        ja->returnCode = 2;
        return;
    }

    for(uint32_t i = 0; i<serverInst->maxPlayers; i++){
        if(serverInst->playerData[i].connection.slotFree) {
            ja->playerID = i;
            ja->secretNumber = (((GetTickCount())*serverInst->currentPlayerCount)%213211)*serverInst->gridH;
            serverInst->playerData[i].connection.addr = *addr;
            serverInst->playerData[i].connection.secretNumber = ja->secretNumber;
            serverInst->playerData[i].connection.slotFree = false;
            serverInst->playerData[i].connection.online = true;

            strcpy_s(serverInst->playerData[i].playerName, 16, jr->playerName);

            serverInst->currentPlayerCount++;

            if(serverInst->currentPlayerCount >= serverInst->playerDataLenght) {
                serverInst->playerDataLenght = serverInst->currentPlayerCount;
            }

            ja->returnCode = 0;
            return;
        }
    }

    ja->returnCode = 2;

}

void removePlayer(LeavePacket* lp, ServerInstance* serverInst) {

    if(lp->playerID >= serverInst->maxPlayers) return;

    PlayerData* player = &serverInst->playerData[lp->playerID];

    if(lp->secretNumber == player->connection.secretNumber) {
        if(player->connection.slotFree) return;
        player->connection.online = false;
        player->connection.slotFree = true;
        player->alive = false;
        serverInst->currentPlayerCount--;

        if(serverInst->playerDataLenght == lp->playerID) {
            serverInst->playerDataLenght--;
        }
    }
}

static void broadcastPacket(GamePacket* gp, int packetSize, ServerInstance* serverInst) {
    if(!isPacketValid(gp)) return;
    if(serverInst->currentPlayerCount == 0) return;
    for(uint32_t i = 0; i<serverInst->playerDataLenght; i++) {
        PlayerData* target = &serverInst->playerData[i];
        if(target->connection.online) {
            sendto(serverInst->serverSocket, gp, packetSize, 0, &target->connection.addr, sizeof(SOCKADDR_IN));
        }
    }

}

void closeServer(ServerInstance* serverInst) {

    TerminateThread(serverInst->hListenerThread, 0);

    GamePacket gp;
    initPacket(&gp, GP_CLOSESERVER);
    broadcastPacket(&gp, sizeof(GamePacket), serverInst);

    closesocket(serverInst->serverSocket);

    HeapFree(hHeap, 0, serverInst->playerData);

    if(serverInst->lanBrdcstData.hThread) {
        TerminateThread(serverInst->lanBrdcstData.hThread, 0);
        closesocket(serverInst->lanBrdcstData.socket);
    }

}

int createServer(ServerInstance* serverInst, WCHAR* port, bool lanVisibility, WCHAR* serverName,
                uint32_t maxPlayers, int32_t gridW, int32_t gridH, bool growing, uint32_t initialLenght,
                int* wsaError) {


    if(hHeap == NULL) {
        hHeap = GetProcessHeap();
    }

    serverInst->serverSocket = INVALID_SOCKET;
    serverInst->serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(serverInst->serverSocket == INVALID_SOCKET) {
        *wsaError = WSAGetLastError();
        return 1;
    }

    ADDRINFOW hints = {};
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_INET;
        hints.ai_protocol = IPPROTO_UDP;
        hints.ai_socktype = SOCK_DGRAM;

    ADDRINFOW* result;
    GetAddrInfoW(NULL, port, &hints, &result);
    if(bind(serverInst->serverSocket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR) {
        *wsaError = WSAGetLastError();
        FreeAddrInfoW(result);
        closesocket(serverInst->serverSocket);
        return 2;
    }


    wcscpy_s(serverInst->wServerName, 64, serverName);
    WideCharToMultiByte(CP_UTF8, 0, serverName, -1, serverInst->serverName, 64, NULL, NULL);

    serverInst->maxPlayers = maxPlayers;

    serverInst->gridW = gridW;
    serverInst->gridH = gridH;

    serverInst->initialPlayerLenght = initialLenght;

    serverInst->playersGrow = growing;

    serverInst->hasGameStarted = FALSE;

    serverInst->currentPlayerCount = 0;
    serverInst->playerData = (PlayerData*)HeapAlloc(hHeap, 0, sizeof(PlayerData)*maxPlayers);
    for(uint32_t i = 0; i<maxPlayers; i++) {
        serverInst->playerData[i].connection.slotFree = true;
        serverInst->playerData[i].connection.online = false;
        serverInst->playerData[i].alive = false;
    }

    if(result->ai_family != AF_INET) {
        FreeAddrInfoW(result);
        return 3;
    }

    FreeAddrInfoW(result);

    serverInst->lanBrdcstData.hThread = NULL;

    if(lanVisibility) {
        wcscpy_s(serverInst->lanBrdcstData.port, 8, port);
        serverInst->lanBrdcstData.hThread = CreateThread(NULL, 0, LanListenerThreadEntry, &serverInst->lanBrdcstData, 0, NULL);
    }

    return 0;

}

DWORD WINAPI LanListenerThreadEntry(LanBroadcastingData* lanBrdcstData) {


    lanBrdcstData->socket = INVALID_SOCKET;
    lanBrdcstData->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(lanBrdcstData->socket == INVALID_SOCKET) {
        MessageBoxW(NULL, L"nigga1", L"fuck", MB_ICONERROR);
        return 1;
    }

    ADDRINFOW hints = {};
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_INET;
        hints.ai_protocol = IPPROTO_UDP;
        hints.ai_socktype = SOCK_DGRAM;

    ADDRINFOW* result;
    GetAddrInfoW(NULL, L"29350", &hints, &result);
    if(bind(lanBrdcstData->socket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR) {
        FreeAddrInfoW(result);
        closesocket(lanBrdcstData->socket);
        return 2;
    }

    LanPeekAnswer lpa;
    initPacket(&lpa.gp, GP_LANPEEK);
    lpa.port = htons((USHORT)_wtoi(lanBrdcstData->port));

    GamePacket buf;

    SOCKADDR_IN fromAddr;
    int fromAddrSize = sizeof(SOCKADDR_IN);

    while(true) {
        recvfrom(lanBrdcstData->socket, &buf, sizeof(GamePacket), 0, &fromAddr, &fromAddrSize);
        if(isPacketValid(&buf)) {
            if(buf.type == GP_LANPEEK) {
                sendto(lanBrdcstData->socket, &lpa, sizeof(LanPeekAnswer), 0, &fromAddr, fromAddrSize);
            }
        }
    }

    return 0;

}
