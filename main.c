#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <string.h>
#include <zconf.h>

#define HTTP_PORT 80
#define NUM_OF_SERVERS 3
#define NUM_OF_CLIENTS 1
#define MIN_PORT_NUM 1024
#define MAX_PORT_NUM 64000
#define MESSAGE_SIZE 128
#define FOUND_END_OF_LINE(c) (*c == '\r') && (*(c+1) == '\n') && (*(c+2) == '\r') && (*(c+3) == '\n')
#define POINTER_EXISTS(c) (c+4)
#define GET_CURR_SERVER(c) ((c + 1) % 3)

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
    int random_port;
    random_port = rand();

    if (random_port < MIN_PORT_NUM || random_port > MAX_PORT_NUM) {
        /* Check if the random port mechanism works appropriately */
        random_port = HTTP_PORT;
    }

    return random_port;
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

    /* Decide case to determine which file to open */
    switch (type) {
        case CLIENT:
            f = fopen(CLIENT_PORT_FILE_NAME, "w");
            break;
        case SERVER:
            f = fopen(SERVER_PORT_FILE_NAME, "w");
            break;
        default:
            break;
    }

    /* Handle failure */
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    /* Write to file */
    fprintf(f, "%d", port);

    fclose(f);
}

void listen_to_socket(int const *socket_fd, int type) {
    if (type == CLIENT) {
        listen(*socket_fd, NUM_OF_CLIENTS);
    }
    else if (type == SERVER) {
        listen(*socket_fd, NUM_OF_SERVERS);
    }
}

void init_socket(int *socket_fd, int type) {
    int port;
    port = get_random_port();

    /* Bind random port */
    while (bind_socket(socket_fd, port) < 0) {
        port = get_random_port();
    }

    /* Write port to txt file */
    write_socket_port_to_file(port, type);

    /* Listen */
    listen_to_socket(socket_fd, type);
}

void init_sockets(int* client_socket, int* server_socket) {
    srand(time(0));

    /* Client */
    init_socket(client_socket, CLIENT);

    /* Server */
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

    return count;
}

void receive_message(char *message, int socket, int type) {
    int num_of_messages = 0, curr_size = 0;

    /*
     * Iterate as long as number of messages found is different from the given type arg:
     * 1 messages = client
     * 2 messages = server
     * */
    while (num_of_messages != type) {
        /* Receive message and current message size */
        curr_size += recv(socket, &(message[curr_size]), MESSAGE_SIZE, 0);

        /* Update number of different messages found in the buffer by searching 'end token' */
        num_of_messages = find_number_of_occurrences(message, "\r\n\r\n");

        /* Update message buffer size */
        message = realloc(message,MESSAGE_SIZE + curr_size);
    }
}

int get_message_size(char *message, int type) {
    int occurrences = 0, size = 0;
    char *temp = message;

    while (POINTER_EXISTS(temp) && occurrences < type) {
        if (FOUND_END_OF_LINE(temp)) {
            occurrences++;
        }
        temp++;
        size++;
    }

    return size+NUM_OF_SERVERS;
}

void handle_message(int receiver, int sender, int type) {
    char *message = malloc(MESSAGE_SIZE);
    int message_size;

    /* Receive */
    receive_message(message, sender, type);

    /* Send */
    message_size = get_message_size(message, type);
    send(receiver, message, message_size, 0);

    free(message);
}

void initiator(int *client_socket, int *server_socket, int *servers) {
    int i;

    /* Get socket number, bind, write to file and listen */
    init_sockets(client_socket, server_socket);

    /* Accept servers connections */
    for (i = 0; i < NUM_OF_SERVERS; i++) {
        servers[i] = accept(*server_socket, NULL, NULL);
    }
}

void executer(int client, int current_server) {
    /* Client ---> LB ---> Server */
    handle_message(current_server, client, CLIENT);

    /* Server ---> LB ---> Client */
    handle_message(client, current_server, SERVER);
}

void load_balancer() {
    int client, client_socket, server_socket, curr_server = -1;
    int servers[NUM_OF_SERVERS];
    int stopped = 0;

    /* Init connections */
    initiator(&client_socket, &server_socket, servers);

    while (!stopped) {
        /* Accept current connection from client */
        client = accept(client_socket, NULL, NULL);

        /* Calculate this way in order to avoid integer overflow */
        curr_server = GET_CURR_SERVER(curr_server);

        /*
         * Execute LB iteration:
         * Client ---> LB ---> Server ---> LB ---> Client
         */
        executer(client, servers[curr_server]);

        /* Close current client connection */
        close(client);
    }
}

int main() {
    load_balancer();
    return 0;
}