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
    short file_status;//-1: not for usage
    				  // 0: eof
    				  // 1: more data
    				  // 2: no file
    				  // 3: request file (client side)
    short data_size;
    char data[MAX_DATASIZE];
};
void error(const char *msg);
void make_pkt(pkt_t *packet, bool SYN, bool ACK, bool FIN, short seq_num, 
				short ack_num, short file_status, short size, char *data);
bool check_pkt (pkt_t *packet);//return true for correct checksum, false for incorrect

