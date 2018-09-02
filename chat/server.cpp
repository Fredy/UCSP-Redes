#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;

int main(void) {
  struct sockaddr_in stSockAddr;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  char buffer[256];
  char protocolDataBuffer[5];
  int n;

  if (-1 == SocketFD) {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;         // el tipo de socket
  stSockAddr.sin_port = htons(2999);       // el puerto
  stSockAddr.sin_addr.s_addr = INADDR_ANY; // la address puede ser cualquiera

  // enlazamos la estructura con el   socket usando bind()
  if (-1 == bind(SocketFD, (const struct sockaddr *)&stSockAddr,
                 sizeof(struct sockaddr_in))) {
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

  // el servidor siempre está corriendo
  while (strcmp(buffer, "quit") != 0) {
    if (0 > ConnectFD) {
      perror("error accept failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    bzero(buffer, 256);               // limpiamos el buffer
    bzero(protocolDataBuffer, 5);
    n = read(ConnectFD, protocolDataBuffer, 5);

    int numChars = stoi(protocolDataBuffer);

    n = read(ConnectFD, buffer, numChars); // leemos
    if (n < 0)
      perror("ERROR reading from socket");
    printf("[CLIENT]: %s", buffer);

    if (strcmp(buffer, "quit") == 0) {
      break;
    }

    bzero(buffer, 256); // limpiamos el buffer
    printf("\n[SERVER]: ");
    // scanf("%s", buffer);
    fgets(buffer, 256, stdin);
    int messageSize = strlen(buffer);
    bzero(protocolDataBuffer, 5); // TODO: this is not necesary
    sprintf(protocolDataBuffer,"%04d", messageSize);

    printf("\n> %s\n", protocolDataBuffer);
    printf("\n. %d\n", messageSize);

    n = write(ConnectFD, protocolDataBuffer, 5);
    n = write(ConnectFD, buffer, messageSize);
    if (n < 0)
      perror("ERROR writing to socket");

    /* perform read write operations ... */
  }
  shutdown(ConnectFD, SHUT_RDWR);

  close(ConnectFD);

  close(SocketFD);
  return 0;
}
