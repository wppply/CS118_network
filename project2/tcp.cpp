#include "tcp.h"


struct pkt_t
{
	bool SYN;
	bool ACK;
	bool FIN;
	int seq_num;
	int ack_num;
	int data_size;
	char data[MAX_DATASIZE];
};

void make_pkt(pkt_t *packet, bool SYN, bool ACK, bool FIN, int seq_num, int size, char *data)
{
	packet->SYN = SYN;
	packet->ACK = ACK;
	packet->FIN = FIN;
	packet->seq_num = seq_num;
	packet->data_size = size;
	if (size !=0 && data != NULL)
		bcopy(data, packet->data, size);
}



