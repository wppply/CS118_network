#define MAX_DATASIZE 1024
typedef struct pkt_t pkt_t;
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
void error(const char *msg);
void make_pkt(pkt_t *packet, bool SYN, bool ACK, bool FIN, int seq_num, int ack_num, int size, char *data);


