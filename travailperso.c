#include <stdio.h>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "Ws2_32.lib")

struct User {
    char id[50];
    char password[50];
};

#define MAX_CLIENTS 10

SOCKET clients[MAX_CLIENTS];
int numClients = 0;

unsigned __stdcall handleClient(void* clientSocket) {
    SOCKET clientSock = *(SOCKET*)clientSocket;
    char buffer[1024];
    int bytesReceived;

    while (1) {
        bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        // Afficher le message reçu côté serveur
        printf("Message received from client: %s\n", buffer);

        // Envoyer le message à tous les clients connectés
        if (strncmp(buffer, "~server", 7) == 0) {
            // Message spécial du serveur, ajoutez le préfixe et envoyez à tous les clients
            for (int i = 0; i < numClients; ++i) {
                send(clients[i], "Server says: ", 1024, 0);
                send(clients[i], buffer + 7, bytesReceived - 7, 0);
            }
        } else {
            // Message d'un client, envoyez à tous les clients
            for (int i = 0; i < numClients; ++i) {
                send(clients[i], buffer, bytesReceived, 0);
            }
        }
    }

    // Retirer le client de la liste
    for (int i = 0; i < numClients; ++i) {
        if (clients[i] == clientSock) {
            for (int j = i; j < numClients - 1; ++j) {
                clients[j] = clients[j + 1];
            }
            numClients--;
            break;
        }
    }

    closesocket(clientSock);
    _endthreadex(0);
    return 0;
}

int main() {
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
            // Ajouter le client à la liste
            clients[numClients] = newSocket;
            numClients++;

            // Créer un thread pour gérer la communication avec le nouveau client
            unsigned threadID;
            _beginthreadex(NULL, 0, &handleClient, &newSocket, 0, &threadID);
        } else {
            // Trop de clients, refuser la connexion
            send(newSocket, "Server full. Please try again later.", 1024, 0);
            closesocket(newSocket);
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
