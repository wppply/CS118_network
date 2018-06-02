#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tcp.h"

class Client
{
    public:
        Client(char *host_name, int port_number);
        ~Client();
        void create_socket();
    private:
        int sockfd, newsockfd, portno;
        char *hostname;
        socklen_t clilen;
        struct sockaddr_in serv_addr, cli_addr;

};




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
}