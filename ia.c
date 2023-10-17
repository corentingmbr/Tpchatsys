#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET sockfd, newSocket;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[1024];
    int addr_size;

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4444);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(sockfd, 5);
    addr_size = sizeof(clientAddr);

    while (1) {
        newSocket = accept(sockfd, (struct sockaddr*)&clientAddr, &addr_size);

        while (1) {
            recv(newSocket, buffer, 1024, 0);
            printf("Message received: %s\n", buffer);
        }

        closesocket(newSocket);
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
