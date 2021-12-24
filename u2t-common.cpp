#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Ws2ipdef.h>

#define MAX_PACKET_SIZE 4096

#pragma comment(lib, "ws2_32.lib")

static void enableTcpNoDelay(SOCKET sock)
{
    BOOL enable = TRUE;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&enable, sizeof(enable));
}

SOCKET createUdpSocket(int localPort)
{
    SOCKET sock;
    int error;
    SOCKADDR_IN addr;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        error = WSAGetLastError();
        printf("socket() failed: %d\n", error);
        return INVALID_SOCKET;
    }

    RtlZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(localPort);
    if (bind(sock, (PSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("bind() failed: %d\n", error);
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

SOCKET createListeningTcpSocket(int localPort)
{
    SOCKET sock;
    int error;
    SOCKADDR_IN addr;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        error = WSAGetLastError();
        printf("socket() failed: %d\n", error);
        return INVALID_SOCKET;
    }

    RtlZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(localPort);
    if (bind(sock, (PSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("bind() failed: %d\n", error);
        closesocket(sock);
        return INVALID_SOCKET;
    }

    if (listen(sock, 1) == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("listen() failed: %d\n", error);
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

SOCKET createConnectedTcpSocket(char* remoteIp, int remotePort)
{
    SOCKET sock;
    int error;
    SOCKADDR_IN addr;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        error = WSAGetLastError();
        printf("socket() failed: %d\n", error);
        return INVALID_SOCKET;
    }

    RtlZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remotePort);
    addr.sin_addr.S_un.S_addr = inet_addr(remoteIp);
    if (connect(sock, (PSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("connect() failed: %d\n", error);
        closesocket(sock);
        return INVALID_SOCKET;
    }

    enableTcpNoDelay(sock);

    return sock;
}

SOCKET createConnectedUdpSocket(int localPort, char* remoteIp, int remotePort)
{
    SOCKET sock;
    SOCKADDR_IN addr;
    int error;

    sock = createUdpSocket(localPort);
    if (sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    RtlZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remotePort);
    addr.sin_addr.S_un.S_addr = inet_addr(remoteIp);
    if (connect(sock, (PSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("connect() failed: %d\n", error);
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

bool readTcpXmitUdpPacket(SOCKET tcpSock, SOCKET udpSock)
{
    unsigned short packetLen;
    int error;
    char buffer[MAX_PACKET_SIZE];

    // Read the packet length header
    error = recv(tcpSock, (char*)&packetLen, sizeof(packetLen), MSG_WAITALL);
    if (error == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("recv() failed: %d\n", error);
        return false;
    }
    else if (error == 0) {
        printf("Remote server disconnected\n");
        return false;
    }

    if (packetLen > sizeof(buffer)) {
        printf("Packet too large to relay: %d\n", packetLen);
        return false;
    }

    // Now the payload
    error = recv(tcpSock, buffer, packetLen, MSG_WAITALL);
    if (error == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("recv() failed: %d\n", error);
        return false;
    }
    else if (error == 0) {
        printf("Remote server disconnected\n");
        return false;
    }

    // And transmit it
    send(udpSock, buffer, packetLen, 0);

    return true;
}

bool readUdpXmitTcpPacket(SOCKET tcpSock, SOCKET udpSock)
{
    unsigned short packetLen;
    int error;
    char buffer[MAX_PACKET_SIZE+sizeof(packetLen)];

    // Read the UDP packet
    error = recv(udpSock, &buffer[sizeof(packetLen)], MAX_PACKET_SIZE, 0);
    if (error == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("recv() failed: %d\n", error);
        return false;
    }

    // Construct the packet header
    packetLen = (unsigned short)error;
    RtlCopyMemory(buffer, &packetLen, sizeof(packetLen));

    // Send the data
    error = send(tcpSock, buffer, sizeof(packetLen)+packetLen, 0);
    if (error == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("send() failed: %d\n", error);
        return false;
    }

    return true;
}

SOCKET acceptSocket(SOCKET tcpServerSock)
{
    int error;

    SOCKET tcpSock = accept(tcpServerSock, NULL, NULL);
    if (tcpSock == INVALID_SOCKET) {
        error = WSAGetLastError();
        printf("accept() failed: %d\n", error);
        return error;
    }

    enableTcpNoDelay(tcpSock);

    return tcpSock;
}

SOCKET pollForRead(SOCKET tcpSock, SOCKET udpSock)
{
    fd_set readfds;
    int error;

    FD_ZERO(&readfds);
    FD_SET(tcpSock, &readfds);
    FD_SET(udpSock, &readfds);

    error = select(0, &readfds, NULL, NULL, NULL);
    if (error == SOCKET_ERROR) {
        error = WSAGetLastError();
        printf("select() failed: %d\n", error);
        return INVALID_SOCKET;
    }

    if (FD_ISSET(tcpSock, &readfds)) {
        return tcpSock;
    }
    if (FD_ISSET(udpSock, &readfds)) {
        return udpSock;
    }

    return INVALID_SOCKET;
}