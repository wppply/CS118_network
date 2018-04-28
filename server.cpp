#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <dirent.h>
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
        const std::string get_contentType(const std::string &filename);
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

void Server::send_404() 
{
    //construct the 404 header
    write(newsockfd, "HTTP/1.1 404 Not Found\n", 14);
    write(newsockfd, "Content-Length: 13\n", 19);
    write(newsockfd, "Content-Type: text/html\n\n", 25);
    write(newsockfd, "<h1>404 Not Found</h1>", 23);
    close(newsockfd);
}

void convert_to_lowercase(char *str)
{
    while (*str) 
    {
        *str = tolower(*str);
        str++;
    }
    return;
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
        std::string filename = parser(buffer);
        //printf("%s\n",filename.c_str());
        process_request(filename);
        // close connection
        close(newsockfd); 
    }
    close(sockfd);
}

const std::string Server::parser(const std::string &rqst)
{
    std::size_t idx_low = rqst.find("/");
    // std::cout << rqst;
    std::size_t idx_high = rqst.find("HTTP/1.1");

    std::string filename;
    if (idx_high-2 <= idx_low+1)
    {
        filename = ":no_file_name";
        return filename;
    }
    filename = rqst.substr(idx_low+1, idx_high-idx_low-2);
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

const std::string Server::get_contentType(const std::string &filename)
{
    std::size_t idx_dot = filename.find_last_of(".");
    std::string extension;
    // find the pos of extension
    if (idx_dot== std::string::npos){
        extension = "";
    }
    else{
        extension = filename.substr(idx_dot + 1);
    }

    std::string Content_Type;

    // define different content type header base on file extension
    if ( (strcasecmp(extension.c_str(), "html") == 0) || 
        (strcasecmp(extension.c_str(), "htm") == 0) ||
        (strcasecmp(extension.c_str(), "txt") == 0)){
        Content_Type = "Content-Type: text/html\n";
    }else if( (strcasecmp(extension.c_str(), "jpg") == 0) || 
        (strcasecmp(extension.c_str(), "jpeg") == 0)){
        Content_Type = "Content-Type: image/jpeg\n";
    }else if( (strcasecmp(extension.c_str(), "gif") == 0)){
        Content_Type = "Content-Type: image/gif\n";
    }else{
        Content_Type = "Content-Type: application/octet-stream\n\n";
    }

    // printf("%s\n",Content_Type.c_str());
    return Content_Type;
}

void Server::process_request(const std::string &filename)
{
    if (filename.compare(":no_file_name") == 0)
    {
        send_404();
        return;
    }
    // find extension to build header
    std::string Content_Type = get_contentType(filename);

    //check directory to find the file to open :flnm
    std::string flnm = "";
    DIR *dp;
    struct dirent *ep;
    dp = opendir(".");
    if (dp == NULL)
    {
        perror ("Couldn't open the directory");
        return;
    }
    while((ep = readdir(dp)))
    {
        char* s1 = strdup(ep->d_name);
        char* s2 = strdup(filename.c_str());
        convert_to_lowercase(s1);
        convert_to_lowercase(s2);
        if(strcmp(s1, s2) == 0)
        {
            flnm = ep->d_name;
        }
        free(s1);
        free(s2);
    }

    // read binary file to form entity body
    if(flnm == "")
    {
        send_404();
        return;
    }
    std::ifstream ifs(flnm, std::ifstream::in | std::ios::binary | std::ios::ate);
    if(!ifs) 
    {
        send_404();
        return;
    }
    std::streampos ifs_size = ifs.tellg();
    char *entity_body = new char[ifs_size];
    ifs.seekg(0, std::ios::beg);
    ifs.read(entity_body, ifs_size);
    ifs.close();

    // create headerlines
    std::string header_lines; 
    header_lines += "HTTP/1.1 200 OK\n";
    header_lines += "Connection: Keep-Alive\n";
    header_lines += "Content-Length: ";
    header_lines += std::to_string(ifs_size);
    header_lines += Content_Type;
    header_lines += "\n";

    //send response message
    write(newsockfd, header_lines.c_str(), header_lines.size());
    write(newsockfd, entity_body, ifs_size);
    delete[] entity_body;
    return;
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