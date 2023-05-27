#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define FILENAME "data.csv"
#define INTERVAL 5 // in seconds

void sendFile(int socket) {
    FILE* file = fopen(FILENAME, "rb");
    if (file == NULL) {
        perror("File error");
        exit(1);
    }

    char buffer[1024];
    int bytesRead = 0;
    while ((bytesRead = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0) {
        if (send(socket, buffer, bytesRead, 0) < 0) {
            perror("Sending error");
            exit(1);
        }
        memset(buffer, 0, sizeof(buffer));
    }

    fclose(file);
    printf("File sent successfully.\n");
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create client socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Socket creation error");
        exit(1);
    }
    printf("Client socket created.\n");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == 0) {
        printf("Connected to server.\n");
    } else {
        perror("Connection error");
        exit(1);
    }

    // Send ID to server
    int id = 123; // Replace with desired ID
    if (send(clientSocket, &id, sizeof(int), 0) < 0) {
        perror("Sending error");
        exit(1);
    }
    printf("Sent ID: %d\n", id);

    while (1) {
        // Send file to server
        sendFile(clientSocket);

        sleep(INTERVAL);
    }

    close(clientSocket);
    return 0;
}