#include "..\u2t-common.hpp"

#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET udpSock;
    SOCKET tcpSock;
    int error;

    if (argc != 5) {
        printf("Usage: %s <local address> <local UDP port> <remote address> <remote TCP port>\n", argv[0]);
        return -1;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("WSAStartup() failed: %d\n", error);
        return error;
    }

    udpSock = createConnectedUdpSocket(0, argv[1], atoi(argv[2]));
    if (udpSock == INVALID_SOCKET) {
        return -1;
    }

    tcpSock = createConnectedTcpSocket(argv[3], atoi(argv[4]));
    if (udpSock == INVALID_SOCKET) {
        return -1;
    }

    for (;;) {
        SOCKET readySocket = pollForRead(tcpSock, udpSock);
        if (readySocket == INVALID_SOCKET) {
            return -1;
        }

        if (readySocket == tcpSock) {
            if (!readTcpXmitUdpPacket(tcpSock, udpSock)) {
                return 0;
            }
        }
        else if (readySocket == udpSock) {
            if (!readUdpXmitTcpPacket(tcpSock, udpSock)) {
                return 0;
            }
        }
    }
}