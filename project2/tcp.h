typedef struct pkt_t pkt_t;
#define MAX_DATASIZE 1024

void make_pkt(pkt_t *packet,PACKET_TYPE type, int seqNum, int size, char *data);


