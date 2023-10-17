#include <stdio.h>
#include <winsock2.h>
#include <process.h>
#include <stdbool.h>

#pragma comment(lib, "Ws2_32.lib")

struct User {
    char id[50];
    char password[50];
};

#define MAX_CLIENTS 10

struct ClientInfo {
    SOCKET socket;
    char username[50];
};

struct ClientInfo clients[MAX_CLIENTS];  // Tableau pour stocker les informations des clients
int numClients = 0;  // Nombre actuel de clients

// Fonction pour gérer la communication avec un client
unsigned __stdcall handleClient(void* clientInfo) {
    struct ClientInfo* info = (struct ClientInfo*)clientInfo;
    SOCKET clientSock = info->socket;
    char buffer[1024];
    int bytesReceived;

    // Authentification
    send(clientSock, "Enter your username: ", 50, 0);
    recv(clientSock, buffer, sizeof(buffer), 0);
    strcpy(info->username, buffer);

    send(clientSock, "Enter your password: ", 50, 0);
    recv(clientSock, buffer, sizeof(buffer), 0);

    // Ajoutez vos propres mécanismes d'authentification ici
    // ...

    while (1) {
        bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            printf("Client %s disconnected.\n", info->username);

            // Retirer le client de la liste
            for (int i = 0; i < numClients; ++i) {
                if (clients[i].socket == clientSock) {
                    // Déplacer les clients suivants vers le haut
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

        // Envoyer le message à tous les clients connectés
        for (int i = 0; i < numClients; ++i) {
            send(clients[i].socket, buffer, bytesReceived, 0);
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
            clients[numClients].socket = newSocket;

            // Créer un thread pour gérer la communication avec le nouveau client
            unsigned threadID;
            _beginthreadex(NULL, 0, &handleClient, &clients[numClients], 0, &threadID);
            numClients++;
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
