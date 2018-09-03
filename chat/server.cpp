#include "common.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>

int main(void) {
  sockaddr_in stSockAddr;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (-1 == SocketFD) {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }

  stSockAddr.sin_family = AF_INET;         // el tipo de socket
  stSockAddr.sin_port = htons(comm::PORT); // el puerto
  stSockAddr.sin_addr.s_addr = INADDR_ANY; // la address puede ser cualquiera

  // enlazamos la estructura con el   socket usando bind()
  if (-1 ==
      bind(SocketFD, (const sockaddr *)&stSockAddr, sizeof(sockaddr_in))) {
    perror("error bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  // ponemos el server en listening mode, para que empieze a escuchar
  // conecciones
  if (-1 == listen(SocketFD, 10)) {
    perror("error listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  // aquí espera por conecciones. // (server socket,...,...) // y retorna el
  // socket del cliente.
  int ConnectFD = accept(SocketFD, NULL, NULL);
  if (0 > ConnectFD) {
    perror("error accept failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  thread thrRead = thread(comm::readThread, ConnectFD, "CLIENT");
  thrRead.detach();
  // el servidor siempre está corriendo
  while (true) {
    cout << "[SERVER]: ";
    string outMessage;
    getline(cin, outMessage);

    comm::writeWithProtocol(outMessage, ConnectFD);

    if (outMessage == comm::QUIT_COMMAND) {
      break;
    }
  }

  shutdown(ConnectFD, SHUT_RDWR);
  close(ConnectFD);
  close(SocketFD);
}
