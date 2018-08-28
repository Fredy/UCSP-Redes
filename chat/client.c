/* Client code in C */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main(void) {
  struct sockaddr_in stSockAddr;
  int Res;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  int n;
  char buffer[256];
  if (-1 == SocketFD) {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(1300);
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

  if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr,
                    sizeof(struct sockaddr_in))) {
    perror("connect failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  while (strcmp(buffer, "EXIT") != 0) {
    bzero(buffer, 256);
    printf("\n[CLIENT]: ");
    scanf("%s", buffer);
    n = write(SocketFD, buffer,
              strlen(buffer)); // eviamos un mensaje junto al tamaño, retorna la
                               // cantidad de bits que se han enviado.
    // en read() retorna cuántos bytes han sido recibidos, porque pueden ser
    // menos de los esperados.
    /* perform read write operations ... */
    if (strcmp(buffer, "EXIT") == 0) {
      break;
    }

    bzero(buffer, 256);
    n = read(SocketFD, buffer, 256);
    printf("[SERVER]: %s", buffer);
  }
  shutdown(SocketFD, SHUT_RDWR);

  close(SocketFD);
  return 0;
}
