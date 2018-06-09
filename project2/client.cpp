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
        bool wait_for_packet();//false for timeout and true for input
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
    make_pkt(&start_con, true, false, false, cli_seq_num, 0, -1, 0, NULL);
    client->cli_seq_num++;
    do
    {
        send_packet(&start_con);
    }
    while (true)
    {}
    //receive SYNACK message from server
    pkt_t recv_con;
    recv_packet(&recv_con);
    serv_seq_num = recv_con.seq_num+1;
    //establish connection
    pkt_t estab_con;
    make_pkt(&start_con, false, true, false, cli_seq_num, serv_seq_num, -1, 0, NULL);
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
    client->cli_seq_num = cal_seq_num(filename_size, client->cli_seq_num);
    pkt_t file_req;
    make_pkt(&file_req, false, true, false, client->cli_seq_num, client->serv_seq_num, 3, filename_size, filename);
    do 
    {
        client->send_packet(&file_req);
    }
    while (!client->wait_for_packet());

    //file 
    FILE *fp;
    int create_file = 1;//flag of creating file.
    int fin_flag = 0;   // for FIN
    pkt_t last = file_req; // last sent packet
    //receive file
    while(1)
    {
        //receive packet
        pkt_t r;
        client->recv_packet(&r);
        if (!check_pkt(&r) || r.ack_num != client->cli_seq_num)//wrong checksum or ack
        {
            do //resend last packet
            {
                client->send_packet(&last);
            }
            while (!client->wait_for_packet());//if timeout then resend again
            continue;                          //if not timeout restart the while loop
        }

        //packet to send
        pkt_t s;
        
        //if server send no_file, set fin
        if (r.file_status == 2) 
        {
            fprintf(stderr, "ERROR: no required file");
            fin_flag = 1;
        }
        else //after handshake if not no_file server only send file
        {
            if (create_file) //only create file once
            {
                FILE *fp = fopen("received.data", "w");
                create_file = 0;
            }
                
            //check data seq
            if (r.seq_num == client->serv_seq_num) //correct packet order
            {
                fwrite(r.data, sizeof(char), r.data_size, fp); // write to file
                if (r.file_status == 0)              //if eof then close file and set fin
                {
                    fclose(fp);
                    fin_flag = 1;
                }
                else                                //if there are more data then make packet
                {
                    client->serv_seq_num = client->cal_seq_num(r.data_size, client->serv_seq_num);
                    make_pkt(&s, false, true, false, client->cli_seq_numm client->serv_seq_num, -1, 0, NULL);
                }
            }
            else if (r.seq_num > client->serv_seq_num) //packet arrives early (out of order)
            {
                //packet keeps the last one
                s = last;
            }
            else //packet arrives late or duplicate
            {
                //ignore this packet, start timer
                if(client->wait_for_packet()) //if receives input continue next loop
                {
                    continue;
                }
                else // if timeout then last packet is going to be sent
                {
                    s = last;
                }
            }
        }
        //if fin is set
        if (fin_flag)
        {
            break;
        }
        // send packet
        do 
        {
            client->send_packet(&s);
        }
        while (!client->wait_for_packet());
        //keep track of last packet
        last = s;
    }
    //start fin
    pkt_t fin;
    make_pkt(&fin, false, false, true, client->cli_seq_num, client->ack_num, -1, 0, NULL);
    while (1)
    {
        client->send_packet(&fin);
        if (client->wait_for_packet())
        {
            pkt_t r;
            client->recv_packet(&r);
            if (r.FIN)
                return;
            else
                continue;
        }     
    }


}


