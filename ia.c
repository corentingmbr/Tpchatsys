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
CRITICAL_SECTION cs; // Ajout d'un verrou pour la synchronisation des threads

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
        EnterCriticalSection(&cs); // Entrer dans la section critique
        if (0 == strncmp(buffer, "~server", 7)) {
            // Message spécial du serveur, ajoutez le préfixe et envoyez à tous les clients
            for (int i = 0; i < numClients; ++i) {
                send(clients[i], "Server says:  ", 1024, 0);
                send(clients[i], buffer + 7, bytesReceived - 7, 0);
            }
        } else {
            // Message d'un client, envoyé à tous les clients
            for (int i = 0; i < numClients; ++i) {
                char* messageCopy = strdup(buffer);
                send(clients[i], messageCopy, bytesReceived, 0);
                free(messageCopy);
            }
        }
        LeaveCriticalSection(&cs); // Quitter la section critique
    }

    // Retirer le client de la liste
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
    _endthreadex(0);
    return 0;
}

int main() {
    InitializeCriticalSection(&cs); // Initialiser le verrou

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
            EnterCriticalSection(&cs);
            clients[numClients] = newSocket;
            numClients++;
            LeaveCriticalSection(&cs);

            // Créer un thread pour gérer la communication avec le nouveau client
            unsigned threadID;
            _beginthreadex(NULL, 0, &handleClient, &newSocket, 0, &threadID);
        } else {
            // Trop de clients, refuser la connexion
            send(newSocket, "Server full. Please try again later.", 1024, 0);
            closesocket(newSocket);
        }
    }

    DeleteCriticalSection(&cs); // Supprimer le verrou
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
