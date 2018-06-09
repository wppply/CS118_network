#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// packet->checksum will be replaced by new calculated checksum
static unsigned long cal_check_sum(pkt_t *packet)  
    {
        packet->check_sum = 0;
        unsigned char *str = (unsigned char *) packet;
        int size = sizeof(pkt_t);
        unsigned long hash = 5381;
        for(int c = 0; c < size; c++)
            hash = ((hash << 5) + hash) + str[c]; /* hash * 33 + c */
        packet->check_sum = hash;
        return hash;
    }

void make_pkt(pkt_t *packet, bool SYN, bool ACK, bool FIN, short seq_num, 
                short ack_num, short file_status, short size, char *data)
{
    packet->SYN = SYN;
    packet->ACK = ACK;
    packet->FIN = FIN;
    packet->seq_num = seq_num;
    packet->ack_num = ack_num;
    packet->file_status = file_status;
    packet->data_size = size;
    if (size > 0 && data != NULL)
    {
        bzero(packet->data, size);
        memcpy(packet->data, data, size);
    }
    
    cal_check_sum(packet);

}

bool check_pkt (pkt_t *packet)
{
    unsigned long cs = packet->check_sum;
    unsigned long computed_cs = cal_check_sum(packet);
    return computed_cs == cs;
}



