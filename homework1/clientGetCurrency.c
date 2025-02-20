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
    char currency1[10], currency2[10];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. Type 'exit' to disconnect.\n");

    while (1) {
        printf("Enter two currencies (e.g., USD EURO): ");
        scanf("%s %s", currency1, currency2);

        if (strcmp(currency1, "exit") == 0 || strcmp(currency2, "exit") == 0) {
            break;
        }

        snprintf(buffer, sizeof(buffer), "%s %s", currency1, currency2);
        send(sock, buffer, strlen(buffer), 0);

        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            printf("Server disconnected.\n");
            break;
        }

        printf("Server response: %s", buffer);
    }

    close(sock);
    printf("Disconnected from server.\n");
    return 0;
}
