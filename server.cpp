#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <regex>
#include <iterator>
#include <iostream>
#include <fstream>


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
        void process_request(const std::string &filename);
        const std::string parser(const std::string &rqst);
        void send_404();

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

        std::string filename = parser(buffer);
        printf("%s\n", filename.c_str());


        process_request(filename);
        // close connection
        close(newsockfd); 
    }
    close(sockfd);
}

const std::string Server::parser(const std::string &rqst)
{
    std::size_t idx_low = rqst.find("/");
    std::size_t idx_high = rqst.find("HTTP/1.1");

    std::string filename = rqst.substr(idx_low+1, idx_high-5);
    // replace all %20 with " "

    std::size_t idx_space = filename.find("%20");
    while(idx_space != std::string::npos)
    {
        // 3 is the length of %20
        filename.replace(idx_space,3," ");
        idx_space = filename.find("%20");
    }
    return filename;

}

void Server::process_request(const std::string &filename)
{
    
    printf("%s\n", "workinig");

    //暂且复制的

    std::ifstream inFile;
    inFile.open(filename, std::ifstream::in | std::ios::binary);
    if(!inFile) {
        send_404();
        return;
    }
    char c;

    std::string response; 
    while(inFile.get(c)) {
        response.push_back(c);
    }
    // find extension to build header

    std::size_t idx_dot = filename.find(".");
    std::string extension;

    

    if (idx_dot== std::string::npos)
    {
        extension = "";
    }
    else
    {
        extension = filename.substr(idx_dot + 1);
    }



    std::string Content_Type;

    if ( (strcasecmp(extension.c_str(), "html") == 0)){
        Content_Type = "Content-Type: text/html\n\n";
    }else if( (strcasecmp(extension.c_str(), "jpg") == 0) || (strcasecmp(extension.c_str(), "jpeg") == 0)){
        Content_Type = "Content-Type: image/jpeg\n\n";
    }else if( (strcasecmp(extension.c_str(), "gif") == 0)){
        Content_Type = "Content-Type: image/gif\n\n";
    }else{
        Content_Type = "Content-Type: application/octet-stream\n\n";
    }


    // send back
    if(inFile.eof()) {
        write(newsockfd, "HTTP/1.1 200 OK\n", 16);
        write(newsockfd, "Content-Length: 13\n", 19);
        write(newsockfd, Content_Type.c_str(), Content_Type.size());
        write(newsockfd, "<h1>Good 200</h1>", 24);
        write(newsockfd, response.c_str(), response.size());
        return;

    } else {
        send_404();
        return;
    }



}

void Server::send_404() {
    //construct the 404 header
    write(newsockfd, "HTTP/1.1 404 Not Found\n", 14);
    write(newsockfd, "Content-Length: 13\n", 19);
    write(newsockfd, "Content-Type: text/html\n\n", 25);
    write(newsockfd, "<h1>404 Not Found</h1>", 23);
    close(newsockfd);
}

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