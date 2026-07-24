#include "Snake.h"

DWORD WINAPI ServerConnectThreadEntry(ServerConnectArgs* args) {

    args->serverConn->clientSocket = INVALID_SOCKET;
    args->serverConn->clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(args->serverConn->clientSocket == INVALID_SOCKET) {
        SendMessageW(hWnd, WM_SERVERCONNECTDONE, 1, 0);
        return 1;
    }

    DWORD timeout = 5000;
    setsockopt(args->serverConn->clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(DWORD));

    GamePacket gp;
    initPacket(&gp, GP_SERVERINFO);

    ServerInfoPacket sip;
    recvfrom(args->serverConn->clientSocket, &sip, sizeof(ServerInfoPacket), 0, NULL, NULL);

    return 0;

}
