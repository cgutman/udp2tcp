#include "..\u2t-common.hpp"

#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET udpSock;
    SOCKET tcpServerSock;
    int error;

    if (argc != 5) {
        printf("Usage: %s <local TCP port> <local UDP port> <remote address> <remote UDP port>\n", argv[0]);
        return -1;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("WSAStartup() failed: %d\n", error);
        return error;
    }

    tcpServerSock = createListeningTcpSocket(atoi(argv[1]));
    if (tcpServerSock == INVALID_SOCKET) {
        return -1;
    }

    udpSock = createConnectedUdpSocket(atoi(argv[2]), argv[3], atoi(argv[4]));
    if (udpSock == INVALID_SOCKET) {
        return -1;
    }

    for (;;) {
        SOCKET tcpSock = acceptSocket(tcpServerSock);
        if (tcpSock == INVALID_SOCKET) {
            return -1;
        }

        for (;;) {
            SOCKET readySocket = pollForRead(tcpSock, udpSock);
            if (readySocket == INVALID_SOCKET) {
                return -1;
            }

            if (readySocket == tcpSock) {
                if (!readTcpXmitUdpPacket(tcpSock, udpSock)) {
                    // This client is gone, break out to accept() again
                    closesocket(tcpSock);
                    break;
                }
            }
            else if (readySocket == udpSock) {
                if (!readUdpXmitTcpPacket(tcpSock, udpSock)) {
                    return 0;
                }
            }
        }
    }
}