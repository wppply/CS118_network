typedef struct pkt_t pkt_t;

int rdt_send (char* data);
pkt_t *make_pkt(char *data);
int udt_send (pkt_t *packet);

int rdt_rcv (pkt_t *packet);
char *extract_pkt(pkt_t *packet);
