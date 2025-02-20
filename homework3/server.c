#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 2
#define ROUNDS 5

typedef enum { ROCK, PAPER, SCISSORS, NONE } Choice;
typedef enum { PLAYER1, PLAYER2 } Player;

typedef struct {
    int socket;
    Choice choice;
    int score;
} Client;

Client clients[MAX_CLIENTS];

void reset_game() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].choice = NONE;
        clients[i].score = 0;
    }
}

void send_message(int client_socket, const char *message) {
    send(client_socket, message, strlen(message), 0);
}

void handle_choice(int player_index, const char *choice_str) {
    if (strcmp(choice_str, "rock") == 0) {
        clients[player_index].choice = ROCK;
    } else if (strcmp(choice_str, "paper") == 0) {
        clients[player_index].choice = PAPER;
    } else if (strcmp(choice_str, "scissors") == 0) {
        clients[player_index].choice = SCISSORS;
    } else {
        clients[player_index].choice = NONE;
    }
}

void determine_winner() {
    if (clients[0].choice == clients[1].choice) {
        clients[0].score++;
        clients[1].score++;
    } else if ((clients[0].choice == ROCK && clients[1].choice == SCISSORS) ||
               (clients[0].choice == SCISSORS && clients[1].choice == PAPER) ||
               (clients[0].choice == PAPER && clients[1].choice == ROCK)) {
        clients[0].score += 2; // Player 1 wins
    } else {
        clients[1].score += 2; // Player 2 wins
    }
}

void play_game() {
    char buffer[1024];
    for (int round = 0; round < ROUNDS; round++) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            send_message(clients[i].socket, "Choose rock, paper, or scissors: ");
            recv(clients[i].socket, buffer, sizeof(buffer), 0);
            handle_choice(i, buffer);
        }

        determine_winner();

        for (int i = 0; i < MAX_CLIENTS; i++) {
            snprintf(buffer, sizeof(buffer), "Round %d results: Player 1 score: %d, Player 2 score: %d\n",
                     round + 1, clients[0].score, clients[1].score);
            send_message(clients[i].socket, buffer);
        }

        reset_game();
    }

    snprintf(buffer, sizeof(buffer), "Final scores: Player 1: %d, Player 2: %d\n",
             clients[0].score, clients[1].score);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        send_message(clients[i].socket, buffer);
    }
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

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        clients[i].socket = client_socket;
        clients[i].score = 0;
        printf("Client %d connected.\n", i + 1);
    }

    // Начало игры
    play_game();

    // Закрытие сокетов
    for (int i = 0; i < MAX_CLIENTS; i++) {
        close(clients[i].socket);
    }
    close(server_socket);
    return 0;
}
