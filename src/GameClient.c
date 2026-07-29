#include "Snake.h"

void sendDirChange(ServerConnection* serverConn, char newDir) {

    DirChangePacket dcp;
    initPacket(&dcp.gp, GP_CHANGEPLAYERDIR);

    dcp.newDir = (uint8_t)newDir;
    dcp.playerID = serverConn->myPlayerID;
    dcp.secretNumber = serverConn->secretNumber;

    sendto(serverConn->clientSocket, &dcp, sizeof(DirChangePacket), 0, &serverConn->addr, sizeof(SOCKADDR_IN));

}

void leaveServer(ServerConnection* serverConn) {

    LeavePacket lp;
    initPacket(&lp.gp, GP_LEAVESERVER);

    lp.playerID = serverConn->myPlayerID;
    lp.secretNumber = serverConn->secretNumber;

    sendto(serverConn->clientSocket, &lp, sizeof(LeavePacket), 0, &serverConn->addr, sizeof(SOCKADDR_IN));

}

DWORD WINAPI ServerJoinThreadEntry(ServerConnectArgs* args) {

    args->serverConn->clientSocket = INVALID_SOCKET;
    args->serverConn->clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(args->serverConn->clientSocket == INVALID_SOCKET) {
        SendMessageW(args->hWnd, WM_SERVERJOINDONE, 1, 0);
        return 1;
    }

    JoinRequestPacket jr;
    initPacket(&jr.gp, GP_JOINREQUEST);

    jr.playerColor = args->playerColor;
    WideCharToMultiByte(CP_UTF8, 0, args->playerName, -1, jr.playerName, 32, NULL, NULL);

    DWORD timeout = 7500;
    setsockopt(args->serverConn->clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(DWORD));

    if(sendto(args->serverConn->clientSocket, &jr, sizeof(JoinRequestPacket), 0, &args->serverConn->addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        SendMessageW(args->hWnd, WM_SERVERJOINDONE, 2, WSAGetLastError());
        closesocket(args->serverConn->clientSocket);
        return 2;
    }

    JoinAnswerPacket ja;
    if(recvfrom(args->serverConn->clientSocket, &ja, sizeof(JoinAnswerPacket), 0, NULL, NULL) == SOCKET_ERROR) {
        SendMessageW(args->hWnd, WM_SERVERJOINDONE, 3, WSAGetLastError());
        closesocket(args->serverConn->clientSocket);
        return 3;
    }

    if(!isPacketValid(&ja.gp) || ja.gp.type != GP_JOINANSWER) {
        SendMessageW(args->hWnd, WM_SERVERJOINDONE, 4, 0);
        closesocket(args->serverConn->clientSocket);
        return 4;
    }

    args->serverConn->myPlayerID = ja.playerID;
    args->serverConn->secretNumber = ja.secretNumber;

    SendMessageW(args->hWnd, WM_SERVERJOINDONE, 0, ja.returnCode);

    timeout = 0;
    setsockopt(args->serverConn->clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(DWORD));

    return 0;

}

DWORD WINAPI LanPeekerThreadEntry(LanPeekThreadArgs* args) {

    *args->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket == INVALID_SOCKET) return 0;

    DWORD timeout = 3500;
    setsockopt(*args->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(DWORD));

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(29350);
    addr.sin_addr.s_addr = htonl(MAKEIPADDRESS(255, 255, 255, 255));

    BOOL broadcasting = TRUE;
    setsockopt(*args->socket, SOL_SOCKET, SO_BROADCAST, &broadcasting, sizeof(BOOL));

    SOCKADDR_IN fromAddr;
    int fromAddrSize = sizeof(SOCKADDR_IN);

    GamePacket gp;
    initPacket(&gp, GP_LANPEEK);

    sendto(*args->socket, &gp, sizeof(GamePacket), 0, &addr, sizeof(SOCKADDR_IN));

    LanGame lanGame;

    LanPeekAnswer lpa;
    while(recvfrom(*args->socket, &lpa, sizeof(LanPeekAnswer), 0, &fromAddr, &fromAddrSize) != SOCKET_ERROR) {
        if(isPacketValid(&lpa.gp)) {
            if(lpa.gp.type == GP_LANPEEK) {
                lanGame.addr = fromAddr;
                lanGame.addr.sin_port = lpa.port;
                SendMessageW(args->hWnd, WM_ADDLANGAME, 0, &lanGame);
            }
        }
    }

    *args->socket = INVALID_SOCKET;

    return 0;

}

DWORD WINAPI ServerConnectThreadEntry(ServerConnectArgs* args) {

    args->serverConn->clientSocket = INVALID_SOCKET;
    args->serverConn->clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(args->serverConn->clientSocket == INVALID_SOCKET) {
        SendMessageW(args->hWnd, WM_SERVERCONNECTDONE, 1, WSAGetLastError());
        return 1;
    }

    DWORD timeout = 5000;
    setsockopt(args->serverConn->clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(DWORD));

    GamePacket gp;
    initPacket(&gp, GP_SERVERINFO);
    if(sendto(args->serverConn->clientSocket, &gp, sizeof(GamePacket), 0, &args->serverConn->addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        SendMessageW(args->hWnd, WM_SERVERCONNECTDONE, 2, WSAGetLastError());
        goto endCloseSocket;
    }

    ServerInfoPacket sip;
    if(recvfrom(args->serverConn->clientSocket, &sip, sizeof(ServerInfoPacket), 0, NULL, NULL) == SOCKET_ERROR) {
        SendMessageW(args->hWnd, WM_SERVERCONNECTDONE, 3, WSAGetLastError());
        goto endCloseSocket;
    }
    if(!isPacketValid(&sip.gp) || sip.gp.type != GP_SERVERINFO) {
        SendMessageW(args->hWnd, WM_SERVERCONNECTDONE, 4, 0);
        goto endCloseSocket;
    }

    SendMessageW(args->hWnd, WM_SERVERCONNECTDONE, 0, &sip);

    endCloseSocket:

    closesocket(args->serverConn->clientSocket);

    return 0;

}
