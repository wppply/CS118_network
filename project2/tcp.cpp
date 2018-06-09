#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


static unsigned long cal_check_sum(pkt_t *packet, int size)
    {
        unsigned char str[MAX_DATASIZE + 6];
        str[0] = (unsigned char) packet->SYN;
        str[1] = (unsigned char) packet->ACK;
        str[2] = (unsigned char) packet->FIN;
        str[3] = (unsigned char) packet->seq_num;
        str[4] = (unsigned char) packet->ack_num;
        str[5] = (unsigned char) packet->file_status;
        str[6] = (unsigned char) packet->size;
        strncat(str, packet->data, sizeof(str));
        
        unsigned long hash = 5381;
        for(int c = 0; c < size; c++)
            hash = ((hash << 5) + hash) + str[c]; /* hash * 33 + c */
        return hash;
    }

void make_pkt(pkt_t *packet, bool SYN, bool ACK, bool FIN, short seq_num, 
                short ack_num, , short file_status, short size, char *data)
{
    packet->SYN = SYN;
    packet->ACK = ACK;
    packet->FIN = FIN;
    packet->seq_num = seq_num;
    packet->ack_num = ack_num;
    packet->file_status = file_status;
    packet->data_size = size;

    bzero(packet->data, size);
    memcpy(packet->data, data, size);
    
    packet->check_sum = cal_check_sum(packet, size);

}

bool check_pkt (pkt_t *packet)
{
    if (packet->data_size == 0)
        return true;
    unsigned long computed_cs = cal_check_sum(packet, packet->data_size);
    return computed_cs == packet->check_sum;
}



