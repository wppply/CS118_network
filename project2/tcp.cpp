#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


static unsigned long cal_check_sum(unsigned char *str, int size)
    {
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
    packet->file_status = file_status;
    packet->data_size = size;
    if (size == 0)
        return;
    if (data != NULL)
    {
        packet->check_sum = cal_check_sum((unsigned char *)data, size);
        bcopy(data, packet->data, size);
    }
    else 
    {
        fprintf(stderr, "%s\n", "Error: Data is NULL while size not equal zero");
        exit(1);
    }
}

bool check_pkt (pkt_t *packet)
{
    if (packet->data_size == 0)
        return true;
    unsigned long computed_cs = cal_check_sum((unsigned char*) packet->data, packet->data_size);
    return computed_cs == packet->check_sum;
}



