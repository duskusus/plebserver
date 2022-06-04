#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <vector>

#define SIZE 1024
#define BACKLOG 999
#define MAXLINE 0xffff
class Request {
  std::string route;
  std::vector<std::string> r;

public:
  Request(std::string request_text) {
    boost::split(r, request_text, boost::is_any_of("\n"));
    std::string line = r[0];
    route = line.substr(4, line.find_first_of("H") - 4);
    std::cout << "Route:" << route << std::endl;
  }
};
class Server {
  int hits = 0;
  struct sockaddr_in sa;
  int socketFD;
  int port;
  std::string header = "HTTP/1.1 200 OK\r\n\r\n";
  char * data;
  std::string response;

public:
  Server(int p_port) {
    port = p_port;
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1) {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  }
  ~Server() { close(socketFD); }
  void processRequest(const std::string &request) {
    // std::cout << request << std::endl;
    Request r(request);
    response = header + data;
    std::cout << response << std::endl;
  }
  void holdResource(std::string file_name) {

    std::ifstream file(file_name, std::ios_base::in);
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    std::cout << size << " Size\n";
    file.seekg(0, std::ios::beg);
    if (!file.read((char *)&data[0], size)) {
      std::cout << "oh no, cant read file!!\n";
      return;
    }

    std::cout << "resource " << data << std::endl;
  }
  void start() {
    if ((int)bind(socketFD, (struct sockaddr *)&sa, sizeof sa) == -1) {
      perror("binding failure");
      close(socketFD);
      exit(EXIT_FAILURE);
    }
    if (listen(socketFD, BACKLOG) == -1) {
      perror("listen failed");
      close(socketFD);
      exit(EXIT_FAILURE);
    }
    while (1) {
      std::cout << "listening\n" << hits << " hits\n";
      char request[MAXLINE]{0};
      unsigned int n = 0;
      int clientSocket = accept(socketFD, NULL, NULL);
      read(clientSocket, request, MAXLINE - 1);
      std::string s_request = request;
      processRequest(s_request);
      send(clientSocket, response.c_str(), response.length(), 0);
      close(clientSocket);
      hits++;
    }
  }
};

int main(int argc, char **argv) {
  if (argv[1] == nullptr) {
    printf("no port provided\n");
    return -1;
  }
  int port = atoi(argv[1]);
  if (port < 1) {
    printf("port out of range\n");
    return -1;
  }
  Server s(port);
  s.holdResource("test.html");
  s.start();
  return 0;
}