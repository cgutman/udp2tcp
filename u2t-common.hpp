#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Ws2ipdef.h>

#define MAX_PACKET_SIZE 4096

SOCKET createUdpSocket(int localPort);
SOCKET createListeningTcpSocket(int localPort);
SOCKET createConnectedTcpSocket(char* remoteIp, int remotePort);
SOCKET createConnectedUdpSocket(int localPort, char* remoteIp, int remotePort);

bool readTcpXmitUdpPacket(SOCKET tcpSock, SOCKET udpSock);
bool readUdpXmitTcpPacket(SOCKET tcpSock, SOCKET udpSock);

SOCKET acceptSocket(SOCKET tcpServerSock);
SOCKET pollForRead(SOCKET tcpSock, SOCKET udpSock);