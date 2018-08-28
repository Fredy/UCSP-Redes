/* Server code in C */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
  struct sockaddr_in stSockAddr;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  char buffer[256];
  int n;

  if (-1 == SocketFD) {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;         // el tipo de socket
  stSockAddr.sin_port = htons(1300);       // el puerto
  stSockAddr.sin_addr.s_addr = INADDR_ANY; // la address puede ser cualquiera

  if (-1 == bind(SocketFD, (const struct sockaddr *)&stSockAddr,
                 sizeof(struct sockaddr_in))) // enlazamos la estructura con el
                                              // socket usando bind()
  {
    perror("error bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  if (-1 == listen(SocketFD, 10)) // ponemos el server en listening mode, para
                                  // que empieze a escuchar conecciones
  {
    perror("error listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  int ConnectFD =
      accept(SocketFD, NULL,
             NULL); // aquí espera por conecciones. // (server socket,...,...)
                    // y retorna el socket del cliente.

  while (strcmp(buffer, "EXIT") != 0) // el servidor siempre está corriendo
  {
    if (0 > ConnectFD) {
      perror("error accept failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    bzero(buffer, 256);               // limpiamos el buffer
    n = read(ConnectFD, buffer, 255); // leemos
    if (n < 0)
      perror("ERROR reading from socket");
    printf("[CLIENT]: %s", buffer);

    if (strcmp(buffer, "EXIT") == 0) {
      break;
    }


    bzero(buffer, 256); // limpiamos el buffer
    printf("\n[SERVER]: ");
    scanf("%s", buffer);
    n = write(ConnectFD, buffer, strlen(buffer));
    if (n < 0)
      perror("ERROR writing to socket");

    /* perform read write operations ... */
  }
  shutdown(ConnectFD, SHUT_RDWR);

  close(ConnectFD);

  close(SocketFD);
  return 0;
}
