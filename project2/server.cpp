#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "tcp.h"
#include <arpa/inet.h>


class Server
{
	public:
		Server(int port_number);
		~Server();
		void setup_server();
		void hand_shake();
        void send_packet(pkt_t *packet);
        void recv_packet(pkt_t *packet);
        void fill_pkt(pkt_t *packet, FILE *file, int len);
        void send_file();
	private:
		int sockfd, newsockfd, portno;
		unsigned long cli_seq_num, serv_seq_num;
		socklen_t clilen;
		struct sockaddr_in serv_addr, cli_addr;
		bool connection;
};

Server::Server(int port_number)
{
	portno = port_number;
	serv_seq_num = 0;
	connection = false;
}

void Server::send_packet(pkt_t *packet)
{
    int sendlen = sendto(sockfd, (char *) packet, sizeof(pkt_t), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (sendlen == -1)
        error("Error: fail to send package");
}

void Server::recv_packet(pkt_t *packet)
{
    socklen_t len;
    int recvlen = recvfrom(sockfd, packet, sizeof(pkt_t), 0, (struct sockaddr *) &serv_addr, &len);
    if (recvlen == -1)
        error("Error: fail to receive package");
}

void Server::fill_pkt(pkt_t *packet, FILE *file, int len)
{

}

void Server::send_file() 
{   



    FILE *file = fopen(file_name, "rb");
    if (file == NULL) 
    {
        error("failed to open file. file not exist")
    }

    struct stat stat_buf;
    int file_size = fstat(fd, &stat_buf);
    file_size == 0 ? stat_buf.st_size : -1;


}



void Server::setup_server() 
{

	// Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // option to prevent ocuppied socket
    // int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    memset((char *) &cli_addr, 0, sizeof(cli_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);


    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // clilen = sizeof(cli_addr);

}
void Server::hand_shake()
{

	// receiving request from server
	pkt_t recv_req;
	socklen_t len;

    int recvlen = recvfrom(sockfd, &recv_req, sizeof(pkt_t), 0, (struct sockaddr *) &serv_addr, &len);
    if (recvlen == -1)
        error("Error: fail to receive package");

    serv_seq_num = recv_req.seq_num;
    cli_seq_num = recv_req.ack_num;

	if (recv_req.SYN && connection == false) 
	{
		// ack the syn
      	pkt_t syn_ack;
	  	make_pkt(&syn_ack, true, false, false, serv_seq_num, ++cli_seq_num, 0, NULL);
        send_packet(syn_ack)
	    //receive ack
	    pkt_t rec_ack;
    	recv_packet(&rec_ack)
        connection = true;
        printf("server: waiting request from client \n");
        // update seq_nums 

    }


}



int main(int argc, char *argv[])
{
	if (argc < 3) 
	{
		fprintf(stderr,"ERROR: Usage: ./sev [port]");
		exit(1);
	}

    int portno = atoi(argv[1]);

    Server *server = new Server(portno);
    server->setup_server();
    server->hand_shake();





}














