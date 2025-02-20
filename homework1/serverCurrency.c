#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define MAX_REQUESTS 5
#define TIMEOUT 60

typedef struct {
    int socket;
    char address[INET_ADDRSTRLEN];
    time_t connect_time;
    int request_count;
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];

void log_connection(ClientInfo client) {
    printf("Client connected: %s at %s", client.address, ctime(&client.connect_time));
}

void handle_client(int client_socket) {
    char buffer[1024];
    char response[1024];
    char currency1[10], currency2[10];
    double rate = 0.0;
    int client_index = -1;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == 0) {
            clients[i].socket = client_socket;
            clients[i].connect_time = time(NULL);
            inet_ntop(AF_INET, &((struct sockaddr_in*)&client_socket)->sin_addr, clients[i].address, sizeof(clients[i].address));
            clients[i].request_count = 0;
            client_index = i;
            break;
        }
    }

    if (client_index == -1) {
        printf("Max clients reached.\n");
        close(client_socket);
        return;
    }

    log_connection(clients[client_index]);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        sscanf(buffer, "%s %s", currency1, currency2);

        // Для примера, просто фиксированные значения курса валют
        if (strcmp(currency1, "USD") == 0 && strcmp(currency2, "EURO") == 0) {
            rate = 0.85; // Пример курса
        } else if (strcmp(currency1, "EURO") == 0 && strcmp(currency2, "USD") == 0) {
            rate = 1.18; // Пример курса
        } else {
            snprintf(response, sizeof(response), "Unknown currencies\n");
            send(client_socket, response, strlen(response), 0);
            continue;
        }

        if (clients[client_index].request_count >= MAX_REQUESTS) {
            snprintf(response, sizeof(response), "Max requests reached. Disconnecting...\n");
            send(client_socket, response, strlen(response), 0);
            break;
        }

        snprintf(response, sizeof(response), "Rate: %.2f\n", rate);
        send(client_socket, response, strlen(response), 0);
        clients[client_index].request_count++;
    }

    printf("Client disconnected: %s\n", clients[client_index].address);
    close(client_socket);
    clients[client_index].socket = 0; // Освобождаем слот
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}
