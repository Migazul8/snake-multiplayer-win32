#include "Snake.h"


DWORD WINAPI ServerJoinThreadEntry(ServerConnectArgs* args) {

    JoinRequestPacket jr;
    initPacket(&jr.gp, GP_JOINREQUEST);

    jr.playerColor = args->playerColor;
    WideCharToMultiByte(CP_UTF8, 0, args->playerName, -1, jr.playerName, 32, NULL, NULL);

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
