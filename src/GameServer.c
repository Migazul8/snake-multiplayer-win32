#include "Snake.h"

static HANDLE hHeap;

void handlePacket(GamePacket* gp, SOCKADDR_IN* fromAddr, ServerInstance* serverInst);

void addPlayer(JoinRequestPacket* jr, ServerInstance* serverInst, SOCKADDR_IN* addr, JoinAnswerPacket* ja);

DWORD WINAPI GameServerThreadEntry(ServerInstance* serverInst) {

    return 0;

}

void handlePacket(GamePacket* gp, SOCKADDR_IN* fromAddr, ServerInstance* serverInst) {
    switch(gp->type) {
        case GP_SERVERINFO:
            if(serverInst->hasGameStarted) return;

            ServerInfoPacket sip;
            initPacket(&sip.gp, GP_SERVERINFO);
            sip.gridW = serverInst->gridW;
            sip.gridH = serverInst->gridH;
            sip.maxPlayers = serverInst->maxPlayers;
            sip.playersGrow = sip.playersGrow;
            wcscpy_s(sip.serverName, 64, serverInst->serverName);

            sendto(serverInst->serverSocket, &sip, sizeof(ServerInfoPacket), 0, fromAddr, sizeof(SOCKADDR_IN));

            break;

        case GP_JOINREQUEST:
            JoinAnswerPacket ja;
            addPlayer((JoinRequestPacket*)gp, serverInst, fromAddr, &ja);

            sendto(serverInst->serverSocket, &ja, sizeof(JoinAnswerPacket), 0, fromAddr, sizeof(SOCKADDR_IN));

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
    }

    if(serverInst->currentPlayerCount >= serverInst->maxPlayers) {
        ja->returnCode = 2;
        return;
    }

    for(uint32_t i = 0; i<serverInst->maxPlayers; i++){
        if(serverInst->playerData[i].connection.slotFree) {
            ja->returnCode = 0;
            ja->playerID = i;
            ja->secretNumber = 67;
            serverInst->playerData[i].connection.addr = *addr;
            serverInst->playerData[i].connection.secretNumber = ja->secretNumber;

            serverInst->currentPlayerCount++;

            if(serverInst->currentPlayerCount > serverInst->playerDataLenght) {
                serverInst->playerDataLenght = serverInst->currentPlayerCount;
            }

            return;
        }
    }

    ja->returnCode = 2;

}

void closeServer(ServerInstance* serverInst) {

    TerminateThread(serverInst->hListenerThread, 0);
    closesocket(serverInst->serverSocket);

    HeapFree(hHeap, 0, serverInst->playerData);

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
    WideCharToMultiByte(CP_UTF8, 0, serverName, -1, serverInst->serverName, 32, NULL, NULL);

    serverInst->maxPlayers = maxPlayers;

    serverInst->gridW = gridW;
    serverInst->gridH = gridH;

    serverInst->hasGameStarted = FALSE;

    serverInst->currentPlayerCount = 0;
    serverInst->playerData = (PlayerData*)HeapAlloc(hHeap, 0, sizeof(PlayerData)*maxPlayers);
    for(uint32_t i = 0; i<maxPlayers; i++) {
        serverInst->playerData[i].connection.slotFree = true;
        serverInst->playerData[i].active = false;
    }

    if(result->ai_family != AF_INET) {
        FreeAddrInfoW(result);
        return 3;
    }

    FreeAddrInfoW(result);

    return 0;

}
