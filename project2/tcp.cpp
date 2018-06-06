#include "tcp.h"

void error(char *msg)
{
    perror(msg);
    exit(1);
}

struct pkt_t
{
    bool SYN;
    bool ACK;
    bool FIN;
    unsigned long seq_num;
    unsigned long ack_num;
    unsigned long check_sum;
    int data_size;
    char data[MAX_DATASIZE];
};

static unsigned long cal_check_sum(unsigned char *str, int size)
    {
        unsigned long hash = 5381;
        for(int c = 0; c < size; c++)
            hash = ((hash << 5) + hash) + str[c]; /* hash * 33 + c */
        return hash;
    }

void make_pkt (pkt_t *packet, bool SYN, bool ACK, bool FIN, int seq_num, int ack_num, int size, char *data)
{
    packet->SYN = SYN;
    packet->ACK = ACK;
    packet->FIN = FIN;
    packet->seq_num = seq_num;
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



