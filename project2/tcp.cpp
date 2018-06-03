#include "tcp.h"


typedef enum {REQ, ACK, DATA, FIN} PACKET_TYPE;

struct pkt_t
{
	PACKET_TYPE type;
	int seqNum;
	int dataSize;
	char data[MAX_DATASIZE];
};

void make_pkt(pkt_t *packet,PACKET_TYPE type, int seqNum, int size, char *data)
{
	packet->type = type;
	packet->seqNum = seqNum;
	packet->dataSize = size;
	bcopy(data, packet->data, size);
}



