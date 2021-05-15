#include "protocol.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <csapp.h>

int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload){
    if (rio_writen(fd, hdr, sizeof(CHLA_PACKET_HEADER)) == -1){
        return -1;
    }
    if (payload != NULL){
        if (rio_writen(fd, payload, ntohl(hdr->payload_length)) == -1){
            return -1;
        }
    }
    return 0;
}

int proto_recv_packet(int fd, CHLA_PACKET_HEADER *hdr, void **payload){
    int read_count = rio_readn(fd, hdr, sizeof(CHLA_PACKET_HEADER));
    if (read_count <= 0){
        return -1;
    }
    if (hdr->payload_length != 0){
        void **buf = (void *)(malloc(sizeof(payload)));
        *payload = buf;
        if (rio_readn(fd, *payload, ntohl(hdr->payload_length)) == -1){
            fprintf(stderr, "Read Error");
        }
    }
    return 0;
}