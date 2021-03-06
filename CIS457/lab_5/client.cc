#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

class Client {
  public:
    const int MAXDATASIZE = 4096;
    int get_port();
    in_addr_t get_host();
    static void* handle_messages(void* arg);
    std::string input_handler();
    int RunClient();
};

void* Client::handle_messages(void* arg) {
  int socket = *(int*) arg;
  char line[5000];
  while (true) {
    recv(socket, line, 5000, 0);
    if (std::strcmp(line, "quit") == 0) {
      std::cout << "Quitting";
      send(socket, line, strlen(line) + 1, 0);
      close(socket);
      return nullptr;
    }
    std::cout << " <<< "<< line << std::flush;
    std::cout << "" << std::endl;
  }
}

int Client::get_port() {
  std::cout << "Enter the port to connect to: " << std::endl;
  std::string port("");
  std::getline(std::cin, port);

  return std::stoi(port);
}

in_addr_t Client::get_host() {
  std::cout << "Enter the host to connect to: " << std::endl;
  std::string host = "";
  std::getline(std::cin, host);

  return inet_addr(host.c_str());
}

std::string Client::input_handler() {
  std::cout << " >>> " << std::flush;
  std::cout << "" << std::endl;
  std::string message("");
  std::getline(std::cin, message);

  // Must be handled as a char*
  return message;
}

int Client::RunClient() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    std::cerr << "There was an error creating the socket!" << std::endl;
    return EXIT_FAILURE;
  }

  int port = get_port();
  in_addr_t host = get_host();
  std::cout << "Client running on localhost:" << port << std::endl;

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = host;

  int c = connect(sockfd, (sockaddr*)&server, sizeof(server));
  if (c < -1) {
    std::cerr << "Error connecting to the server" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "Connected successfully" << std::endl;

  pthread_t child;
  pthread_create(&child, nullptr, Client::handle_messages, &sockfd);
  pthread_detach(child);

  while (true) {
    std::string message = input_handler();
    if (message == "quit") {
      std::cout << "Exiting" << std::endl;
      send(sockfd, message.c_str(), message.length() + 1, 0);
      close(sockfd);
      return EXIT_SUCCESS;
    }
    send(sockfd, message.c_str(), message.length() + 1, 0);
  }

  return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
  Client c;
  c.RunClient();

  return EXIT_SUCCESS;
}
