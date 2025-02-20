#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define MAX_REQUESTS 10
#define TIME_LIMIT 3600 // 1 час в секундах
#define INACTIVITY_LIMIT 600 // 10 минут в секундах

typedef struct {
    struct sockaddr_in address;
    time_t last_active;
    int request_count;
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];

void log_request(const char *client_ip, const char *request) {
    FILE *log_file = fopen("server_log.txt", "a");
    if (log_file) {
        fprintf(log_file, "Client: %s, Request: %s, Time: %s", client_ip, request, ctime(&(time_t){time(NULL)}));
        fclose(log_file);
    }
}

void handle_request(int sock, struct sockaddr_in *client_addr, char *buffer) {
    char response[1024];
    char *ingredients = buffer;
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, sizeof(client_ip));

    int client_index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].address.sin_addr.s_addr == client_addr->sin_addr.s_addr &&
            clients[i].address.sin_port == client_addr->sin_port) {
            client_index = i;
            break;
        }
    }

    if (client_index == -1) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].request_count == 0) {
                clients[i].address = *client_addr;
                clients[i].last_active = time(NULL);
                clients[i].request_count = 0;
                client_index = i;
                break;
            }
        }
    }

    if (client_index == -1) {
        snprintf(response, sizeof(response), "Server busy. Try again later.\n");
        sendto(sock, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    time_t current_time = time(NULL);
    if (current_time - clients[client_index].last_active > INACTIVITY_LIMIT) {
        clients[client_index].request_count = 0;
    }

    if (clients[client_index].request_count >= MAX_REQUESTS) {
        snprintf(response, sizeof(response), "Request limit reached. Try again later.\n");
        sendto(sock, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        return;
    }

    clients[client_index].last_active = current_time;
    clients[client_index].request_count++;

    // Для примера, просто фиксированный ответ
    snprintf(response, sizeof(response), "Recipes with %s: Recipe1, Recipe2, Recipe3\n", ingredients);

    log_request(client_ip, ingredients);

    sendto(sock, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
}

int main() {
    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &addr_len);
        if (bytes_received < 0) {
            perror("Receive failed");
            continue;
        }
        handle_request(sock, &client_addr, buffer);
    }

    close(sock);
    return 0;
}
