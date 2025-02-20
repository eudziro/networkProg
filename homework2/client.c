#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1" // IP-адрес сервера

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("Connected to server. Type 'exit' to disconnect.\n");

    while (1) {
        printf("Enter ingredients (comma-separated): ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Удаление символа новой строки

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

        memset(buffer, 0, sizeof(buffer));
        socklen_t addr_len = sizeof(server_addr);
        int bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &addr_len);
        if (bytes_received < 0) {
            perror("Receive failed");
            break;
        }

        printf("Server response: %s", buffer);
    }

    close(sock);
    printf("Disconnected from server.\n");
    return 0;
}
