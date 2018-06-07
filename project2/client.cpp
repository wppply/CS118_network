#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "tcp.h"


class Client
{
    public:
        Client(char *host_name, int port_number);
        ~Client();
        void create_socket();
        void send_packet(pkt_t *packet);
        void recv_packet(pkt_t *packet);
        int hand_shake();
        short cal_seq_num(int add_val, short seq_num);//calculate new seq number
        short cli_seq_num, serv_seq_num;
    private:
        int sockfd, portno;
        char *hostname;
        struct hostent *server;
        struct sockaddr_in serv_addr;
        struct pollfd fds[1];
};

Client::Client(char *host_name, int port_number)
{
    hostname = host_name;
    portno = port_number;
    cli_seq_num = 0;
    server = NULL;

}

short Client::cal_seq_num(int add_val, short seq_num)
{
    long s = (long) add_val + (long) seq_num;
    if (s > MAX_SEQ_NUM)
    {
        s = s - MAX_SEQ_NUM;
    }
    return (short) s;
}

void Client::send_packet(pkt_t *packet)
{
    int sendlen = sendto(sockfd, (char *) packet, sizeof(pkt_t), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (sendlen == -1)
        error("Error: fail to send package");
}

void Client::recv_packet(pkt_t *packet)
{
    socklen_t len;
    int recvlen = recvfrom(sockfd, packet, sizeof(pkt_t), 0, (struct sockaddr *) &serv_addr, &len);
    if (recvlen == -1)
        error("Error: fail to receive package");
}

void Client::create_socket()
{
    //create client side socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sockfd < 0)                     
        error("ERROR opening socket\n");
    //fill in address info
    server = gethostbyname(hostname);
    if (server == NULL) 
        error("ERROR, no such host\n");
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    //setup poll and timeout
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
}
bool Client::wait_for_packet()
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

int Client::hand_shake()
{
    //send SYN=1 special message
    pkt_t start_con;
    make_pkt(&start_con, true, false, false, cli_seq_num, 0, 0, NULL);
    send_packet(&start_con);
    //receive SYNACK message from server
    pkt_t recv_con;
    recv_packet(&recv_con);
    serv_seq_num = recv_con.seq_num+1;
    //establish connection
    pkt_t estab_con;
    make_pkt(&start_con, false, true, false, ++cli_seq_num, serv_seq_num, 0, NULL);
    send_packet(&estab_con);
    //establishment successful
    return 1;
}


int main(int argc, char** argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "ERROR: Usage: ./client [hostname] [port] [filename]");
        exit(1);
    }
    char *hostname = argv[1];
    int portno = atoi(argv[2]);
    char *filename = argv[3];
    //setup connection
    Client *client = new Client(hostname, portno);
    client->create_socket();
    client->hand_shake();
    //send file request
    int filename_size = strlen(filename) + 1;
    unsigned long updated_seq_num = client->cli_seq_num + filename_size;
    pkt_t file_req;
    make_pkt(&file_req, false, true, false, updated_seq_num, client->serv_seq_num, filename_size, filename);
    client->send_packet(&file_req);
    //receive file
    while(1)
    {

    }

}


