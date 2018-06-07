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

typedef struct pkt_t pkt_t;

class Server
{
	public:
		Server(int port_number);
		~Server();
		void setup_server();
		void hand_shake();
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
      	cli_seq_num = recv_req.ack_num + 1;
      	serv_seq_num = recv_req.seq_num;
	  	make_pkt(&syn_ack, true, false, false, serv_seq_num, cli_seq_num, 0, NULL);

	    if (sendto(sockfd, &syn_ack, sizeof(pkt_t), 0, (struct sockaddr *) &cli_addr, clilen) < 0) 
		    printf("Error writing SYN-ACK packet\n"); 
	    //receive ack
	    pkt_t rev_ack;
    	if (recvfrom(sockfd, &rev_ack, sizeof(pkt_t), 0, (struct sockaddr *) &serv_addr, &len) < 0)
        	error("Error: fail to receive package");
        serv_seq_num = rev_ack.seq_num;
    	cli_seq_num = rev_ack.ack_num;
        connection = true;

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














