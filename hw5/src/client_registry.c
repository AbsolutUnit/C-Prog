#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <globals.h>
#include <unistd.h>


typedef struct client_registry
{
    CLIENT* clients[MAX_CLIENTS + 1];
} CLIENT_REGISTRY;

pthread_mutex_t locker;

CLIENT_REGISTRY *creg_init(){
    CLIENT_REGISTRY* new_reg = calloc(1, sizeof(struct client_registry));
    for (int i = 0; i < MAX_CLIENTS + 1; i++){
        new_reg->clients[i] = NULL;
    }
    return new_reg;
}

void creg_fini(CLIENT_REGISTRY *cr){
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (cr->clients[i] != NULL){
            free(cr->clients[i]);
        }
    }
}

CLIENT *creg_register(CLIENT_REGISTRY *cr, int fd){
    pthread_mutex_lock(&locker);
    CLIENT* new_client = client_create(cr, fd);
    client_ref(new_client, "created");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (cr->clients[i] == NULL) {
            cr->clients[i] = new_client;
            pthread_mutex_unlock(&locker);
            return new_client;
        }
    }
    pthread_mutex_unlock(&locker);
    return NULL;
}

int creg_unregister(CLIENT_REGISTRY *cr, CLIENT *client){
    return 0;
}

CLIENT **creg_all_clients(CLIENT_REGISTRY *cr){
    return cr->clients;
}

void creg_shutdown_all(CLIENT_REGISTRY *cr){
    return;
}