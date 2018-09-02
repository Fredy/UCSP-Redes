#include "common.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(void) {
  sockaddr_in stSockAddr;
  int Res;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (-1 == SocketFD) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(comm::PORT);
  Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);

  if (0 > Res) {
    perror("error: first parameter is not a valid address family");
    close(SocketFD);
    exit(EXIT_FAILURE);
  } else if (0 == Res) {
    perror("char string (second parameter does not contain valid ipaddress");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  if (-1 ==
      connect(SocketFD, (const sockaddr *)&stSockAddr, sizeof(sockaddr_in))) {
    perror("connect failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  while (true) {
    cout << "[CLIENT]: ";
    string outMessage;
    getline(cin, outMessage);

    comm::writeWithProtocol(outMessage, SocketFD);

    if (outMessage == comm::QUIT_COMMAND) {
      break;
    }

    const string inMessage = comm::readWithProtocol(SocketFD);
    cout << "[SERVER]: " << inMessage << '\n';

    if (inMessage == comm::QUIT_COMMAND) {
      break;
    }
  }

  shutdown(SocketFD, SHUT_RDWR);
  close(SocketFD);
}
