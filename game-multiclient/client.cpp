#include "common.hpp"
#include "ui.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>

int main(void) {
  // ------ Connection Setup ------
  sockaddr_in stSockAddr;
  int Res;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (-1 == SocketFD) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(comm::PORT);
  //Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
  Res = inet_pton(AF_INET, "192.168.43.58", &stSockAddr.sin_addr);

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

  // ------ UI Setup ------
  NcursesUI ui;
  //ui.init();

  // ------ Actual functionality ------
  atomic<bool> shouldClose(false);
  thread thrRead = thread(comm::readConcurrent, SocketFD, "SERVER", ref(ui), ref(shouldClose));
  thrRead.detach();
  //string Move;
  while (true) {
     //cout << "[CLIENT]: ";
    //string outMessage = ui.readInput();
    //ui.writeOutput("[CLIENT]: " + outMessage);
    //cin>> Move ;
    string outMessage;
    getline(cin, outMessage);

    if (shouldClose) {
      break;
    }

    comm::writeWithProtocol(outMessage, SocketFD);

    if (outMessage == comm::QUIT_COMMAND) {
      break;
    }
  }
  //ui.terminate();
  cout << "Disconnected\n";

  // ------ Connection Shutdown ------
  shutdown(SocketFD, SHUT_RDWR);
  close(SocketFD);
}
