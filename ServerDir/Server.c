#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define FILENAME_LENGTH 100

void saveFile(int file_no, long file_size, int socket) {
    printf("Saving file #%d start.\n", file_no);
    char file_name[FILENAME_LENGTH];
    sprintf(file_name, "received_%d.csv", file_no);

    FILE* file = fopen(file_name, "wb");
    if (file == NULL) {
        perror("File error");
        exit(1);
    }

    char buffer[1024];
    int bytesRead = 0;
    long bytesReadTotal = 0;
    while ((bytesRead = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        if (bytesRead < 0) {
            perror("Receiving error");
        }
        fwrite(buffer, sizeof(char), bytesRead, file);
        memset(buffer, 0, sizeof(buffer));
        bytesReadTotal += bytesRead;
        printf("%ld\n", bytesReadTotal);
        if (bytesReadTotal >= file_size) {
            break;
        }
    }

    fclose(file);
    printf("File '%s' received and saved successfully.\n", file_name);
}

int main() {
    printf("Program Starting...\n");

    int serverSocket, newSocket;
    struct sockaddr_in serverAddr, newAddr;
    socklen_t addrSize;

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

    printf("Program Successfully Started\n");

    // Listening for incoming connections
    if (listen(serverSocket, 10) < 0) {
        perror("Listening error");
        exit(1);
    }

    addrSize = sizeof(newAddr);
    int fileReceived = 0;  // Variable to track the number of files received

    while (fileReceived < 1) {  // Exit the loop when one file is received
        // Accept connection from client
        newSocket = accept(serverSocket, (struct sockaddr*)&newAddr, &addrSize);
        if (newSocket < 0) {
            perror("Accepting error");
            exit(1);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        // Receive ID from client
        char id[15] = "";
        if (recv(newSocket, id, sizeof(id), 0) < 0) {
            perror("Receiving error");
            exit(1);
        }
        printf("Received ID: %s\n", id);
        while (1) {
            // Receive File Name from client
            int file_no;
            if (recv(newSocket, &file_no, sizeof(file_no), 0) < 0) {
                perror("Receiving error");
                exit(1);
            }
            printf("File #%d received.\n", file_no);
            // Receive File Size from client
            long file_size;
            if (recv(newSocket, &file_size, sizeof(file_size), 0) < 0) {
                perror("Receiving error");
                exit(1);
            }
            printf("File Size : %ld\n", file_size);
            // Receive file from client
            saveFile(file_no, file_size, newSocket);
        }

        close(newSocket);
    }

    close(serverSocket);
    return 0;
}