#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <string.h>
#include <zconf.h>

#define NUM_OF_SERVERS 3
#define MIN_PORT_NUM 1024
#define MAX_PORT_NUM 64000
#define MESSAGE_SIZE 100
#define ONE 1
#define TWO 2
#define THREE 3
#define FOUR 4

#define SERVER_PORT_FILE_NAME "server_port"
#define CLIENT_PORT_FILE_NAME "http_port"

/****************************************************
*                                                   *
*                C Language Workshop                *
*                Sockets Assignment                 *
*                                                   *
*****************************************************/

typedef enum {
    CLIENT=1,
    SERVER
} socket_e;

int get_random_port() {
    int r;
    r = rand();

    while (r < MIN_PORT_NUM || r > MAX_PORT_NUM) {
        r = rand();
    }

    return r;

}
int bind_socket(int *socket_fd, int port) {
    struct sockaddr_in service;

    *socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (*socket_fd < 0) {
        return -1;
    }

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(port);

    return bind(*socket_fd ,(struct sockaddr *)&service, sizeof(service));
}

void write_socket_port_to_file(int port, int type) {
    FILE *f;

    if (type == CLIENT) {
        f = fopen(CLIENT_PORT_FILE_NAME, "w");
    } else {
        f = fopen(SERVER_PORT_FILE_NAME, "w");
    }

    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    fprintf(f, "%d", port);
    fclose(f);

}

void listen_to_socket(int const *socket_fd, int type) {
    if (type == CLIENT) {
        listen(*socket_fd, ONE);
    }
    else if (type == SERVER) {
        listen(*socket_fd, THREE);
    }
}

void init_socket(int *socket_fd, int type) {
    int port;
    port = get_random_port();


    /* Bind random port */
    while (bind_socket(socket_fd, port) != 0) {
        port = get_random_port();
    }

    /* Write port to txt file */
    write_socket_port_to_file(port, type);

    /* Listen */
    listen_to_socket(socket_fd, type);
}

void init_sockets(int* client_socket, int* server_socket) {
    srand(time(0));

    init_socket(client_socket, CLIENT);
    init_socket(server_socket, SERVER);
}

int find_number_of_occurrences(char const *string, char *substring) {
    int count = 0;
    const char *tmp = string;
    tmp = strstr(tmp, substring);
    while(tmp) {
        count++;
        tmp++;
        tmp = strstr(tmp, substring);
    }

    // printf("%s: found %d occurrences\n", __func__, count);
    return count;
}

char *get_message(int socket, int type) {
    char *message = malloc(MESSAGE_SIZE);
    int num_of_messages = 0, curr_size = 0;

    while (num_of_messages != type) {
        curr_size += recv(socket, &(message[curr_size]), MESSAGE_SIZE, 0);
        num_of_messages = find_number_of_occurrences(message, "\r\n\r\n");
        message = realloc(message,MESSAGE_SIZE + curr_size);
    }

    // printf("%s: size of message is %d, type is %d\n",__func__,curr_size,type);
    return message;
}

int get_message_size(char *message, int type) {
    int occurrences = 0, size = 0;
    char *temp = message;

    while (temp+FOUR && occurrences < type) {
        if ((*temp == '\r') && (*(temp+ONE) == '\n') && (*(temp+TWO) == '\r') && (*(temp+THREE) == '\n')) {
            occurrences++;
        }
        temp++;
        size++;
    }

    return size+THREE;
}

void send_message(int receiver, int sender, int type) {
    int message_size;
    char *message;

    message = get_message(sender, type);
    //printf("_%d_message: %s\n",type,message);
    message_size = get_message_size(message, type);

    send(receiver, message, message_size, 0);
    free(message);
}

void load_balancer() {
    int client, client_socket, server_socket, i, iterations = 0, curr_server;
    int servers[NUM_OF_SERVERS];
    int stopped = 0;

    /* Get socket number, bind, write to file and listen */
    init_sockets(&client_socket, &server_socket);

    for (i = 0; i < NUM_OF_SERVERS; i++) {
        servers[i] = accept(server_socket, NULL, NULL);
    }

    while (!stopped) {
        curr_server = servers[iterations % NUM_OF_SERVERS];

        /* Accept connection from client */
        client = accept(client_socket, NULL, NULL);

        /* Client ---> LB ---> Server */
        send_message(curr_server, client, CLIENT);

        /* Server ---> LB ---> Client */
        send_message(client, curr_server, SERVER);

        close(client);
        iterations++;
    }
}

int main() {
    load_balancer();
    return 0;
}