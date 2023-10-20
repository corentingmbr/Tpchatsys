#include <stdio.h>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

SOCKET clients[MAX_CLIENTS];
int numClients = 0;

CRITICAL_SECTION cs;
typedef struct {
    const char* login;
    const char* password;
} Credentials;

Credentials allowedCredentials[MAX_CLIENTS] = {
        {"user1", "pass1"},
        {"user2", "pass2"},
};

DWORD WINAPI handleClient(void* clientSocket) {
    SOCKET clientSock = *(SOCKET*)clientSocket;
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    bytesReceived = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        printf("Client disconnected.\n");
        closesocket(clientSock);
        return 0;
    }
    buffer[bytesReceived] = '\0';
    const char* receivedLogin = buffer;
    printf("login: %s\n", receivedLogin);

    bytesReceived = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        printf("Client disconnected.\n");
        closesocket(clientSock);
        return 0;
    }
    buffer[bytesReceived] = '\0';
    const char* receivedPassword = buffer;
    printf("Pass: %s\n", receivedPassword);

    int validCredentials = 0;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (strcmp(receivedLogin, allowedCredentials[i].login) == 0 &&
            strcmp(receivedPassword, allowedCredentials[i].password) == 0) {
            validCredentials = 1;
            break;
        }
    }
    if (!validCredentials) {
        printf("Invalid credentials. Disconnecting client.\n");
        closesocket(clientSock);
        return 0;
    }

    while (1) {
        bytesReceived = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        buffer[bytesReceived] = '\0';
        printf("Message received from client: %s\n", buffer);

        EnterCriticalSection(&cs);

        for (int i = 0; i < numClients; ++i) {
            if (clients[i] != clientSock) {
                send(clients[i], buffer, bytesReceived, 0);
            }
        }

        LeaveCriticalSection(&cs);
    }

    EnterCriticalSection(&cs);
    for (int i = 0; i < numClients; ++i) {
        if (clients[i] == clientSock) {
            for (int j = i; j < numClients - 1; ++j) {
                clients[j] = clients[j + 1];
            }
            numClients--;
            break;
        }
    }
    LeaveCriticalSection(&cs);

    closesocket(clientSock);
    return 0;
}

int main() {
    InitializeCriticalSection(&cs);

    WSADATA wsaData;
    SOCKET sockfd, newSocket;
    struct sockaddr_in serverAddr, clientAddr;
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

    printf("Server listening on port 4444...\n");

    while (1) {
        newSocket = accept(sockfd, (struct sockaddr*)&clientAddr, &addr_size);

        if (numClients < MAX_CLIENTS) {
            EnterCriticalSection(&cs);
            clients[numClients] = newSocket;
            numClients++;
            LeaveCriticalSection(&cs);

            HANDLE thread_handle = CreateThread(NULL, 0, handleClient, &clients[numClients - 1], 0, NULL);
            CloseHandle(thread_handle);
        } else {
            send(newSocket, "Server full. Please try again later.", BUFFER_SIZE, 0);
            closesocket(newSocket);
        }
    }

    DeleteCriticalSection(&cs);
    closesocket(sockfd);
    WSACleanup();
    return 0;
}

