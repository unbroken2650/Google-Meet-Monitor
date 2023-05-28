#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define FILENAME "received_data.csv"
#define INTERVAL 5 // in seconds

void saveFile(char* data, int length) {
    FILE* file = fopen(FILENAME, "wb");
    if (file == NULL) {
        perror("File error");
        exit(1);
    }

    fwrite(data, sizeof(char), length, file);
    fclose(file);
    printf("File saved successfully.\n");
}

int main() {
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr, newAddr;
    socklen_t addrSize;
    char buffer[1024];

    // Creating Server Socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation error");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Binding socket to IP/Port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Binding error");
        exit(1);
    }

    // Listening for incoming connections
    if (listen(serverSocket, 10) < 0) {
        perror("Listening error");
        exit(1);
    }

    addrSize = sizeof(newAddr);

    while (1) {
        // Accept connection from client
        newSocket = accept(serverSocket, (struct sockaddr*)&newAddr, &addrSize);
        if (newSocket < 0) {
            perror("Accepting error");
            exit(1);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        // Receive ID from client
        memset(buffer, 0, sizeof(buffer));
        char id[15] = "";
        int idLength = recv(newSocket, &id, sizeof(id), 0);
        if (idLength < 0) {
            perror("Receiving error");
            exit(1);
        }
        printf("Received ID: %s\n", id);

        // Receive file from client
        memset(buffer, 0, sizeof(buffer));
        int fileLength = recv(newSocket, buffer, sizeof(buffer), 0);
        if (fileLength < 0) {
            perror("Receiving error");
            exit(1);
        }
        printf("Received file data.\n");

        // Save file
        saveFile(buffer, fileLength);

        close(newSocket);
        sleep(INTERVAL);
    }

    close(serverSocket);
    return 0;
}
