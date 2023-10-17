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

    // ... (Code d'authentification)

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
    // ... (Votre code existant)

    while (1) {
        newSocket = accept(sockfd, (struct sockaddr*)&clientAddr, &addr_size);

        if (numClients < MAX_CLIENTS) {
            // Ajouter le client à la liste
            clients[numClients].socket = newSocket;
            strcpy(clients[numClients].username, "Guest");  // Remplacez cela par le vrai nom d'utilisateur après l'authentification
            numClients++;

            // Créer un thread pour gérer la communication avec le nouveau client
            unsigned threadID;
            _beginthreadex(NULL, 0, &handleClient, &clients[numClients - 1], 0, &threadID);
        } else {
            // Trop de clients, refuser la connexion
            send(newSocket, "Server full. Please try again later.", 1024, 0);
            closesocket(newSocket);
        }
    }

    // ... (Votre code existant)

    return 0;
}











































#include <stdio.h>
#include <winsock2.h>
#include <process.h>  // Pour _beginthreadex

#pragma comment(lib, "Ws2_32.lib")

struct User {
    char id[50];
    char password[50];
};

// Fonction pour gérer la communication avec un client
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

        printf("Message received from client: %s\n", buffer);
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

        // Créer un thread pour gérer la communication avec le nouveau client
        unsigned threadID;
        _beginthreadex(NULL, 0, &handleClient, &newSocket, 0, &threadID);
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}





#include <stdio.h>
#include <winsock2.h>
#include <process.h>
#include <stdbool.h>

#pragma comment(lib, "Ws2_32.lib")

struct User {
    char id[50];
    char password[50];
};

// Fonction pour gérer la communication avec un client
unsigned __stdcall handleClient(void* clientSocket) {
    SOCKET clientSock = *(SOCKET*)clientSocket;
    char buffer[1024];
    int bytesReceived;

    // Ajouter une vérification d'authentification
    struct User users[] = {
            {"user1", "pass1"},
            {"user2", "pass2"},
            // Ajoutez d'autres utilisateurs au besoin
    };
    int numUsers = sizeof(users) / sizeof(users[0]);

    bool authenticated = false;

    // Authentification
    send(clientSock, "Enter your username: ", 50, 0);
    recv(clientSock, buffer, sizeof(buffer), 0);
    char username[50];
    strcpy(username, buffer);

    send(clientSock, "Enter your password: ", 50, 0);
    recv(clientSock, buffer, sizeof(buffer), 0);
    char password[50];
    strcpy(password, buffer);

    for (int i = 0; i < numUsers; ++i) {
        if (strcmp(users[i].id, username) == 0 && strcmp(users[i].password, password) == 0) {
            authenticated = true;
            break;
        }
    }

    if (!authenticated) {
        send(clientSock, "Authentication failed. Closing connection.", 50, 0);
        closesocket(clientSock);
        _endthreadex(0);
        return 0;
    } else {
        send(clientSock, "Authentication successful. You can now chat.", 50, 0);
    }

    // Boucle de chat
    while (1) {
        bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        // Ajouter une vérification de commande
        if (buffer[0] == '/') {
            // Si le message commence par '/', considérez-le comme une commande
            // Ajoutez d'autres commandes au besoin
            if (strcmp(buffer, "/help\n") == 0) {
                send(clientSock, "Available commands:\n/help - Show help\n/quit - Disconnect from the server\n", 1024, 0);
            } else if (strcmp(buffer, "/quit\n") == 0) {
                send(clientSock, "You are now disconnected from the server.\n", 1024, 0);
                closesocket(clientSock);
                _endthreadex(0);
                return 0;
            } else {
                send(clientSock, "Invalid command. Type /help for available commands.\n", 1024, 0);
            }
        } else {
            printf("Message received from client %s: %s\n", username, buffer);
        }
    }

    closesocket(clientSock);
    _endthreadex(0);
    return 0;
}

int main() {
    // ... (Votre code existant)

    while (1) {
        newSocket = accept(sockfd, (struct sockaddr*)&clientAddr, &addr_size);

        // Créer un thread pour gérer la communication avec le nouveau client
        unsigned threadID;
        _beginthreadex(NULL, 0, &handleClient, &newSocket, 0, &threadID);
    }

    // ... (Votre code existant)

    return 0;
}


//#include <stdio.h>
//#include <winsock2.h>
//
//#pragma comment(lib, "Ws2_32.lib")
//
//
//struct User {
//char id[50];
//char password[50];
//
//};
//
//
//
//int main() {
//    WSADATA wsaData;
//    SOCKET sockfd, newSocket;
//    struct sockaddr_in serverAddr, clientAddr;
//    char buffer[1024];
//    int addr_size;
//
//    WSAStartup(MAKEWORD(2, 2), &wsaData);
//    sockfd = socket(AF_INET, SOCK_STREAM, 0);
//    memset(&serverAddr, 0, sizeof(serverAddr));
//
//    serverAddr.sin_family = AF_INET;
//    serverAddr.sin_port = htons(4444);
//    serverAddr.sin_addr.s_addr = INADDR_ANY;
//
//    bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
//    listen(sockfd, 5);
//    addr_size = sizeof(clientAddr);
//
//    while (1) {
//        newSocket = accept(sockfd, (struct sockaddr*)&clientAddr, &addr_size);
//
//        while (1) {
//            recv(newSocket, buffer, 1024, 0);
//            printf("Message received: %s\n", buffer);
//        }
//
//        closesocket(newSocket);
//    }
//
//    closesocket(sockfd);
//    WSACleanup();
//    return 0;
//}