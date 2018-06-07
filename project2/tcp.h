#define MAX_DATASIZE 1024
#define TIME_OUT 500
#define MAX_SEQ_NUM 30719
#define WINDOW_SIZE 5120
typedef struct pkt_t pkt_t;
struct pkt_t
{
    bool SYN;
    bool ACK;
    bool FIN;
    short seq_num;
    short ack_num;
    unsigned long check_sum;
    short data_size;
    char data[MAX_DATASIZE];
};
void error(const char *msg);
void make_pkt(pkt_t *packet, bool SYN, bool ACK, bool FIN, int seq_num, int ack_num, int size, char *data);
bool check_pkt (pkt_t *packet);

