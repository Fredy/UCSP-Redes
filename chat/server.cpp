#include "common.hpp"
#include "ui.hpp"
#include <arpa/inet.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
int main(void) {
  // ------ Connection Setup ------
  sockaddr_in stSockAddr;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (-1 == SocketFD) {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }

  stSockAddr.sin_family = AF_INET;         // Socket type.
  stSockAddr.sin_port = htons(comm::PORT); // Setting the port.
  stSockAddr.sin_addr.s_addr = INADDR_ANY; // The address could be anything.

  // Bind the structure with the socket using bind()
  if (-1 ==
      bind(SocketFD, (const sockaddr *)&stSockAddr, sizeof(sockaddr_in))) {
    perror("error bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  // Setting the server to listening mode so it can listen conections.
  if (-1 == listen(SocketFD, 10)) {
    perror("error listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  // The server waits for connections and returns the client socket.
  int ConnectFD = accept(SocketFD, NULL, NULL);
  if (0 > ConnectFD) {
    perror("error accept failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  // ------ UI Setup ------
  NcursesUI ui;
  ui.init();

  // ------ Actual functionality ------
  thread thrRead = thread(comm::readConcurrent, ConnectFD, "CLIENT", ref(ui));
  thrRead.detach();

  while (true) {
    // cout << "[SERVER]: ";
    string outMessage = ui.readInput();
    ui.writeOutput("[SERVER]: " + outMessage);
    // getline(cin, outMessage);

    comm::writeWithProtocol(outMessage, ConnectFD);

    if (outMessage == comm::QUIT_COMMAND) {
      break;
    }
  }
  ui.terminate();
  cout << "Disconnected\n";

  // ------ Connection Shutdown ------
  shutdown(ConnectFD, SHUT_RDWR);
  close(ConnectFD);
  close(SocketFD);
}
