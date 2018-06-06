#define MAX_DATASIZE 1024
typedef struct pkt_t pkt_t;

void make_pkt(pkt_t *packet, bool SYN, bool ACK, bool FIN, int seq_num, int size, char *data);


