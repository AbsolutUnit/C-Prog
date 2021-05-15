#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <globals.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

typedef struct client
{
    USER* user;
    int ref_count;
    int fd;
    MAILBOX* mail;
    int logged;
} CLIENT;

static pthread_mutex_t locker;

CLIENT *client_create(CLIENT_REGISTRY *creg, int fd){
    CLIENT* new_client = malloc(sizeof(CLIENT));
    new_client->fd = fd;
    new_client->user = NULL;
    new_client->ref_count = 1;
    new_client->logged = 0;
    return new_client;
}

CLIENT *client_ref(CLIENT *client, char *why){
    client->ref_count += 1;
    return client;
}

void client_unref(CLIENT *client, char *why){
//    pthread_mutex_lock(&locker);
    client->ref_count -= 1;
    if (client->ref_count == 0){
        free(client->user);
        free(client);
        client = NULL;
    }
//    pthread_mutex_unlock(&locker);
}

int client_login(CLIENT *client, char *handle){
    USER* new_user = ureg_register(user_registry, handle);
    pthread_mutex_lock(&locker);
    CLIENT** all_clients = creg_all_clients(client_registry);
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (all_clients[i] == NULL){
            continue;
        } else {
            if (all_clients[i]->user == NULL){
                continue;
            } else {
                if (strcmp(user_get_handle(client_get_user(all_clients[i], 1)), handle) == 0 && all_clients[i]->logged == 1) {
                    pthread_mutex_unlock(&locker);
                    return -1;
                }
            }
        }
    }
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (all_clients[i] == NULL){
            continue;
        } else if (all_clients[i]->fd == client->fd && all_clients[i]->logged == 0) {
            all_clients[i]->fd = client->fd;
            all_clients[i]->user = new_user;
            all_clients[i]->ref_count = client->ref_count;
            all_clients[i]->logged = 1;
            all_clients[i]->mail = mb_init(handle);
            mb_ref(all_clients[i]->mail, "set");

            pthread_mutex_unlock(&locker);
            return 0;
        }
    }
//    fprintf(stdout, "\nHello end loop 1\n");
//    CLIENT* new_client = creg_register(client_registry, client->fd);
//    fprintf(stdout, "\nHang here\n");
//    fprintf(stdout, "\nHang there\n");
//    new_client->fd = client->fd;
//    new_client->user = new_user;
//    fprintf(stdout, "\nUser addr %p\n", new_client->user);
//    new_client->mail = mb_init(handle);
//    mb_ref(new_client->mail, "set");
//    fprintf(stdout, "\nHello start loop 2 with mailbox %p\n", new_client->mail);
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (all_clients[i] == NULL){
            all_clients[i] = creg_register(client_registry, client->fd);
            all_clients[i]->fd = client->fd;;
            all_clients[i]->user = new_user;
//            all_clients[i]->ref_count = new_client->ref_count;
            all_clients[i]->logged = 1;
//            fprintf(stdout, "\nDad did we make it\n");
            all_clients[i]->mail = mb_init(handle);
            all_clients[i]->mail = mb_init(handle);
            mb_ref(all_clients[i]->mail, "set");

//            all_clients[i] = new_client;
            pthread_mutex_unlock(&locker);
            return 0;
        }
    }
    pthread_mutex_unlock(&locker);
    return -1;
}

int client_logout(CLIENT *client){
//    pthread_mutex_lock(&locker);
    CLIENT** all_clients = creg_all_clients(client_registry);
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (strcmp(user_get_handle(client_get_user(all_clients[i], 1)), user_get_handle(client->user)) != 0 && i == MAX_CLIENTS - 1){
            pthread_mutex_unlock(&locker);
            return -1;
        } else if (strcmp(user_get_handle(client_get_user(all_clients[i], 1)), user_get_handle(client->user)) == 0){
            user_unref(client->user, "Client logout");
            client->user = NULL;
            mb_unref(client->mail, "Client logout");
            mb_shutdown(client->mail);
            client = NULL;
            all_clients[i] = NULL;
//            pthread_mutex_unlock(&locker);
            return 0;
        }
    }
//    pthread_mutex_unlock(&locker);
    return -1;
}

USER *client_get_user(CLIENT *client, int no_ref){
    return client->user;
}

MAILBOX *client_get_mailbox(CLIENT *client, int no_ref){
    return client->mail;
}

int client_get_fd(CLIENT *client){
    return client->fd;
}

int client_send_packet(CLIENT *user, CHLA_PACKET_HEADER *pkt, void *data){
    return proto_send_packet(user->fd, pkt, data);
}

int client_send_ack(CLIENT *client, uint32_t msgid, void *data, size_t datalen){
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    CHLA_PACKET_HEADER* new_packet = malloc(sizeof(CHLA_PACKET_HEADER));
    memset(new_packet, 0, sizeof(CHLA_PACKET_HEADER));
    new_packet->type = CHLA_ACK_PKT;
    new_packet->payload_length = htonl(datalen);
    new_packet->msgid = htonl(msgid);
    new_packet->timestamp_sec = current_time.tv_sec;
    new_packet->timestamp_nsec = (current_time.tv_sec * 1000000000) + (current_time.tv_usec * 1000);
    return client_send_packet(client, new_packet, data);
}

int client_send_nack(CLIENT *client, uint32_t msgid){
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    CHLA_PACKET_HEADER* new_packet = malloc(sizeof(CHLA_PACKET_HEADER));
    memset(new_packet, 0, sizeof(CHLA_PACKET_HEADER));
    new_packet->type = CHLA_NACK_PKT;
    new_packet->payload_length = 0;
    new_packet->msgid = htonl(msgid);
    new_packet->timestamp_sec = current_time.tv_sec;
    new_packet->timestamp_nsec = (current_time.tv_sec * 1000000000) + (current_time.tv_usec * 1000);
    return client_send_packet(client, new_packet, NULL);
}