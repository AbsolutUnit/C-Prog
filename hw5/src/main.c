#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <csapp.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "debug.h"
#include "server.h"
#include "globals.h"

static void terminate(int);

/*
 * "Charla" chat server.
 *
 * Usage: charla <port>
 */
int main(int argc, char* argv[]){

    int c;
    char* port;
    int sock;
    int flag = 1;
//    int debug = 0;
//    int automated = 0;
//    char* hostname = "127.0.0.1";
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    // Perform required initializations of the client_registry and
    // player_registry.
    while ((c = getopt(argc, argv, "dqh:p:")) != -1){
        switch (c) {
            case 'h':
//                hostname = optarg;
            case 'p':
                port = optarg;
                fprintf(stdout, "Hello World");
            case 'd':
//                debug = 0;
            case 'q':
                continue;
//                automated = 1;
        }
    }
    struct sockaddr_in new_add;
    socklen_t siz = sizeof(struct sockaddr_in);
    new_add.sin_port = htons(atoi(port));
    sock = open_listenfd(port);
    if (sock == -1){
        perror("Binding error");
    }

    user_registry = ureg_init();
    client_registry = creg_init();

    pthread_t threading;
    while (flag){
        int* client_soc = malloc(sizeof(int));
        *client_soc = Accept(sock, (SA *)&new_add, &siz);
        if (pthread_create(&threading, NULL, chla_client_service, client_soc) < 0){
            perror("Thread creation failed.");
        }
    }

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function charla_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    fprintf(stderr, "You have to finish implementing main() "
	    "before the server will function.\n");

    terminate(EXIT_FAILURE);
}

void handler(int sig) {
    terminate(EXIT_SUCCESS);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shut down all existing client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    // Finalize modules.
    creg_fini(client_registry);
    ureg_fini(user_registry);

    debug("%ld: Server terminating", pthread_self());
    exit(status);
}
