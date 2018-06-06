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

void error(char *msg)
{
    perror(msg);
    exit(1);
}

class Server
{
	public:
		Server(char *host_name, int port_number);
		~Server();
		void hand_shake();
	private:
		int sockfd, newsockfd, portno
		unsigned long cli_seq_num, serv_seq_num;
		socklen_t clilen;
		char *hostname;
		struct sockaddr_in serv_addr, cli_addr;
		bool connection = false;
}

Server::Server(char *host_name, int port_number)
{
	hostname = host_name;
	portno = port_number;
	ser_seq_num = 0;
}
void Server::setup_server() 
{

	if (argc < 3) 
	{
		fprintf(stderr,"ERROR: Usage: ./sev [hostname] [port]");
		exit(1);
	}
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // create socket
	if (sockfd < 0)
        error("ERROR opening socket");

    // option to prevent ocuppied socket
    // int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

    //
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(hostname)//INADDR_ANY;// redefine the ip host here
    serv_addr.sin_port = htons(portno);


    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    clilen = sizeof(cli_addr);


}
void Server::hand_shake()
{
	listen(sockfd, 5);  // 5 simultaneous connection at most

	

	char buffer[8193]; //buffer 大小怎么定？
	memset(buffer, 0, 8193);

	// int n = recvfrom(sockfd, buffer, 8193, 0, (struct sockaddr *) &cli_addr, &clilen);// pos mean receive data

	struct pkt_t clientPacket;
	buffer2pkt(&clientPacket, buffer);

	if (clientPacket.SYN && connection == false) {
      struct pkt_t syn_ack_pkt;

      syn_ack_pkt.seq_num = 0; 
      syn_ack_pkt.SYN = true;
      syn_ack_pkt.ack_num = clientPacket.ack_num + 1; 
      if (sendto(sock_fd, &syn_ack_pkt, sizeof(struct pkt_t), 0, 
      			(struct sockaddr *) &client_addr, cli_len) > 0) 
      {
            printf("Sending packet %d SYN\n", (syn_ack_pkt.seq_num));
            connection = true;
      }
      else {
        printf("Error writing SYN-ACK packet\n"); 
      }
    }

}



int main(int argc, char *argv[])
{
	if (argc < 3) 
	{
		fprintf(stderr,"ERROR: Usage: ./sev [hostname] [port]");
		exit(1);
	}

	char *hostname = argv[1];
    int portno = atoi(argv[2]);

    Server *server = new Server(hostname, portno);
    server.setup_server();
    server.hand_shake();



}














