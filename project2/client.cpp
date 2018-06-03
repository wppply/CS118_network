#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "tcp.h"

void error(char *msg)
{
    perror(msg);
    exit(0);
}

class Client
{
    public:
        Client(char *host_name, int port_number);
        ~Client();
        void create_and_connect_socket();
        int hand_shake();
        int request_file(char *file_name);
    private:
        int sockfd, newsockfd, portno;
        char *hostname;
        struct sockaddr_in serv_addr;

};

Client::Client(char *host_name, int port_number)
{
	hostname = host_name;
	portno = port_number;

}

void Client::create_and_connect_socket()
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

    //establish a connection to the server
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
}

int Client::hand_shake()
{

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
    
    Client *client = new Client(hostname, portno);
    client.create_socket();
    client.hand_shake();
    while(1)
    {
    	
    }

}


