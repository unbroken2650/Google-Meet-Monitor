#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define INTERVAL 5  // in seconds
#define FILENAME_LENGTH 100

long calculateFileSize(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("File error");
        return -1;
    }

    fseek(file, 0, SEEK_END);  // Move the file pointer to the end of the file
    long size = ftell(file);   // Get the current position of the file pointer (which is the file size)
    fclose(file);

    return size;
}

void sendFile(int file_no, int socket) {
    char file_name[FILENAME_LENGTH];
    sprintf(file_name, "%d.csv", file_no);

    char command[500] = "";
    sprintf(command, "touch %s", file_name);
    system(command);

    // Packet Capture
    printf("Capturing Packets...\n");

    system("touch output.pcap");
    system("chmod 777 output.pcap");
    system("sudo tshark -a duration:5 -i 1 -f \"udp\" -w output.pcap");

    strcpy(command, "");  // command set to ""
    sleep(5);             // Delay for 5 seconds

    char subcommand[300];
    strcpy(subcommand, "sudo tshark -r output.pcap -T fields -E header=y -E separator=, -E quote=d ");
    strcat(command, subcommand);
    strcpy(subcommand, "-e frame.number -e frame.time_epoch -e ip.src -e ip.dst -e udp.srcport -e udp.dstport -e data.data ");
    strcat(command, subcommand);
    sprintf(subcommand, "-e rtcp.pt -e rtcp.ssrc.fraction > %s", file_name);
    strcat(command, subcommand);
    // Concatenate the file_name directly within the sprintf command
    system(command);

    FILE* file = fopen(file_name, "rb");

    if (file == NULL) {
        perror("File error");
        exit(1);
    }

    long fileSize = calculateFileSize(file_name);
    if (send(socket, &fileSize, sizeof(fileSize), 0) < 0) {
        perror("Sending error");
        exit(1);
    }
    printf("File Sent (%ld)\n", fileSize);

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
    char id[15] = "";  // Replace with desired ID
    printf("Enter Meet ID: ");
    scanf("%s", id);
    if (send(clientSocket, &id, sizeof(id), 0) < 0) {
        perror("Sending error");
        exit(1);
    }
    printf("Sent ID: %s\n", id);

    int file_no = 1;

    while (1) {
        // Send file to server
        if (send(clientSocket, &file_no, sizeof(file_no), 0) < 0) {
            perror("Sending error");
            exit(1);
        }
        printf("Sending File #%d...\n", file_no);
        sendFile(file_no++, clientSocket);

        printf("File #%d Sent.\n", file_no);

        sleep(INTERVAL);
    }

    close(clientSocket);
    return 0;
}