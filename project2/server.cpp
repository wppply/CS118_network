#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "tcp.h"
#include <arpa/inet.h>
#include <math.h>
#include <unistd.h>
#include <poll.h>

class Server
{
	public:
		Server(int port_number);
		~Server();
		void setup_server();
		void hand_shake();
        void send_fin(pkt_t *data_pkt);
        void send_packet(pkt_t *packet);
        void recv_packet(pkt_t *packet);

        void fill_pkt(pkt_t *data_pkt, int pkt_next_seq, int num_packet, FILE *file, int file_size);
        void send_file(pkt_t *recv_pkt);
        bool wait_for_packet();//false for timeout and true for input
        short cal_seq_num(int add_val, short seq_num);//calculate new seq number
        short cli_seq_num, serv_seq_num;
        bool connection;
	private:
		int sockfd, newsockfd, portno;
		socklen_t clilen;
		struct sockaddr_in serv_addr, cli_addr;
		char* data_buffer;
        struct pollfd fds[1];
};

Server::Server(int port_number)
{
	portno = port_number;
	serv_seq_num = 0;
	connection = false;
}
Server::~Server()
{
    free(data_buffer);
    close(sockfd);
}

short Server::cal_seq_num(int add_val, short seq_num)
{
    long s = (long) add_val + (long) seq_num;
    if (s > MAX_SEQ_NUM)
    {
        s = s - MAX_SEQ_NUM;
    }
    return (short) s;
}

void Server::send_packet(pkt_t *packet)
{
    int sendlen = sendto(sockfd, (char *) packet, sizeof(pkt_t), 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
    printf("");
    // printf("send packet\n");
    if (sendlen == -1)
        error("Error: fail to send package");
}

void Server::recv_packet(pkt_t *packet)
{
    // socklen_t len;
    int recvlen = recvfrom(sockfd, (char *) packet, sizeof(pkt_t), 0, (struct sockaddr *) &cli_addr, &clilen);
    // printf("receive packet\n");
    if (recvlen == -1)
        error("Error: fail to receive package");
}





void Server::setup_server() 
{
	// Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    memset((char *) &cli_addr, 0, sizeof(cli_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);


    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    //setup poll and timeout
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    //set up data buffer
    data_buffer = (char *) calloc(MAX_DATASIZE, sizeof(char));
}

bool Server::wait_for_packet()
{
    int ret = poll(fds, 1, TIME_OUT);
    if (ret == -1)
        error("Error: poll");
    if (ret == 0)
        return false;
    if (fds[0].revents & POLLIN)
        return true;
    return false;
}

void Server::hand_shake()
{

    // receiving request from server
    pkt_t recv_req;
    // first SYN
    recv_packet(&recv_req);
    printf("1st syn received: Seq%d ACK%d \n",recv_req.seq_num, recv_req.ack_num);


    cli_seq_num = recv_req.seq_num;

    if (recv_req.SYN && connection == false) 
    {

        // ack the syn
        pkt_t syn_ack; 
        cli_seq_num = cal_seq_num(1,cli_seq_num);
        make_pkt(&syn_ack, true, true, false, serv_seq_num, cli_seq_num, -1, 0, NULL);//need to change
        // retransmission if reply is lost or timeout
        do
        {
            send_packet(&syn_ack);
            printf("1st syn sent: Seq%d ACK%d \n",syn_ack.seq_num, syn_ack.ack_num);
        }
        while (!wait_for_packet());
        // update seq num
        serv_seq_num = cal_seq_num(1,serv_seq_num);

        //receive ack
        recv_packet(&syn_ack);
        printf("2rd received syn: Seq%d ACK%d \n",syn_ack.seq_num, syn_ack.ack_num);
        if (syn_ack.ack_num == serv_seq_num){
            connection = true;
            printf("server: waiting request from client \n");
        }
        
    }
}


void Server::send_fin(pkt_t *data_pkt){
    printf("1st received FIN from client\n");
    //ack
    cli_seq_num = cal_seq_num(1,cli_seq_num);
    make_pkt(data_pkt, false, true, false, serv_seq_num, cli_seq_num, -1, 0, NULL);
    send_packet(data_pkt);
    printf("1st sent ACK FIN: Seq%d ACK%d \n",data_pkt->seq_num, data_pkt->ack_num);

    //fins
    serv_seq_num = cal_seq_num(1,serv_seq_num);
    make_pkt(data_pkt, false, false, true, serv_seq_num, cli_seq_num, -1, 0, NULL);
    printf("2rd sent FIN: Seq%d ACK%d \n",data_pkt->seq_num, data_pkt->ack_num);
    send_packet(data_pkt);

    connection = false;

}


void Server::fill_pkt(pkt_t *data_pkt, int pkt_next_seq, int num_packet, FILE *file, int file_size)
{   
    int status;

    // figure out data_size
    if (pkt_next_seq == num_packet) 
    {
        data_pkt->data_size = file_size % MAX_DATASIZE;
        status = 0;
    }else{
        data_pkt->data_size = MAX_DATASIZE;
        status = 1;
    }
    
    fseek(file, (pkt_next_seq-1) * MAX_DATASIZE, SEEK_SET);//-1 
    fread(data_buffer, sizeof(char), data_pkt->data_size, file);
    printf("%lu byte data has been filled into packet\n", sizeof(*data_buffer) );

    // maybe need to minus 1 here
    int seq_temp = cal_seq_num((pkt_next_seq-1) * MAX_DATASIZE,serv_seq_num);
    make_pkt(data_pkt, false, true, false, seq_temp, cal_seq_num(1,cli_seq_num), status, data_pkt->data_size, data_buffer);

    
}


void Server::send_file(pkt_t *recv_pkt) 
{   
    pkt_t data_pkt, ack_pkt;
    // because the seq may over flow, define annother for packet number
    int pkt_cur_seq = 1;
    int pkt_next_seq = 1;
    char *file_name = recv_pkt->data;
    

    FILE *file = fopen(file_name, "rb");
    if (file == NULL) 
    {
        error("failed to open file. file not exist");
        // no file, try to fin the connection
        make_pkt(&data_pkt, false, false, true, serv_seq_num, cal_seq_num(1,recv_pkt->seq_num), 2, 0, NULL);
        send_packet(&data_pkt);

    }

    
    // find how long the file is
    struct stat stat_buf;
    int rc = stat(file_name, &stat_buf);
    int file_size = (rc == 0) ? stat_buf.st_size : -1;
    //how many packet we need to send totally
    int num_packet = ceil(file_size / MAX_DATASIZE);


    while(connection)//(pkt_cur_seq <= num_packet )
    {

        int pkt_temp_seq = pkt_next_seq;//double pointers to control window

        for(int i=0; (i < pkt_cur_seq+WINDOW_SIZE/MAX_DATASIZE - pkt_temp_seq) && pkt_next_seq<=num_packet;i++)
        {

            fill_pkt(&data_pkt,pkt_next_seq,num_packet,file,file_size); 
            send_packet(&data_pkt);
            printf("sent %d bytes, pak_num: %d, SEQ: %d , ACK: %d \n", data_pkt.data_size, pkt_next_seq, data_pkt.seq_num, data_pkt.ack_num);
            pkt_next_seq++;

        }

        // start to check fin
        if (wait_for_packet())
        {
            recv_packet(&ack_pkt);
            if(ack_pkt.ACK && ack_pkt.ack_num == cal_seq_num(pkt_cur_seq * MAX_DATASIZE, serv_seq_num)) {

                printf("ACK: %d received, currentSeq: %d\n", ack_pkt.ack_num, ack_pkt.seq_num);
                // cli_seq_num = cal_seq_num(ack,ack_pkt.ack_num)  ack_pkt.seq_num ;
                // serv_seq_num = cal_seq_num(MAX_DATASIZE, serv_seq_num);
                pkt_cur_seq++;
            }
            else if (ack_pkt.FIN) // waiting not 
            {
                printf("FIN: %d received, currentSeq: %d\n", ack_pkt.ack_num, ack_pkt.seq_num);
                send_fin(&data_pkt);

            }
            else
            {
                printf("FAIL: %d received, currentSeq: %d\n", ack_pkt.ack_num, ack_pkt.seq_num);
                pkt_next_seq = pkt_cur_seq;
            }
            

        }else{ // timeout

            printf("ACK: %d received, currentSeq: %d\n", ack_pkt.ack_num, ack_pkt.seq_num);
            pkt_next_seq = pkt_cur_seq;

        }
    }

}




int main(int argc, char *argv[])
{
	if (argc < 2) 
	{
		fprintf(stderr,"ERROR: Usage: ./sev [port]");
		exit(1);
	}

    int portno = atoi(argv[1]);
    

    Server *server = new Server(portno);
    
    server->setup_server();
    printf("server starting to work\n");
    

    while (1){

        printf("waiting for hand_shake\n");
        server->hand_shake();


        pkt_t recv_pkt;
        if (server->connection)
        {
            printf("wating for request\n");
            server->recv_packet(&recv_pkt);
            if(recv_pkt.file_status == 3)
                server->cli_seq_num = server->cal_seq_num(recv_pkt.data_size, server->cli_seq_num);
                server->send_file(&recv_pkt);
        }
        else
        {
            printf("failed to hand_shake\n");
            exit(0);
        }

    }



    return 0;

}














