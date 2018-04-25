#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

class Server
{
    public:
        Server(int port_number);
        ~Server();
        void create_socket();
        void server_listen();
    private:
        int sockfd, newsockfd, portno;
        socklen_t clilen;
        struct sockaddr_in serv_addr, cli_addr;
        void error(const char *msg);
        void process_request(char *rqst);

};

Server::Server(int port_number)
{
    portno = port_number;
}

Server::~Server()
{
    close(sockfd);
    close(newsockfd);
}

void Server::error(const char *msg)
{
    perror(msg);
    exit(1);
}

void Server::create_socket()
{
    //create server side socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }
    //fill in address info
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }
}

void Server::server_listen()
{
    if (listen(sockfd, 16) < 0) //16 simultaneous connections at most
    {
        error("Error on listening");    
    }
    while (true)//main accept loop
    {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
        {
            error("ERROR on accept");
        }
        int n;
        char buffer[8193];
        memset(buffer, 0, 8193);
        //read client request message
        n = read(newsockfd, buffer, 8193);
        if (n < 0) 
        {
            error("ERROR reading from socket");
        }
        //process the client request message
        printf("%s\n", buffer);
        process_request(buffer);
        // close connection
        close(newsockfd); 
    }
    close(sockfd);
}

void Server::process_request(char *rqst)
{}

Server* server = NULL;

void sig_handler(int sig)
{
    if (server != NULL)
    {
        delete server;
    }
}

int main(int argc, char** argv)
{
    signal(SIGINT, sig_handler);
    int portno;
    if (argc < 2) 
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    else
    {
        portno = atoi(argv[1]);
    }
    server = new Server(portno);
    server->create_socket();
    server->server_listen();
    delete server;
    return 0;
}