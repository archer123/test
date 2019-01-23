
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#define PACKETSIZE 2000


int main() {

  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(5000);
  //server.sin_port = htons(5100);
  server.sin_addr.s_addr = htons(INADDR_ANY);


  int fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0) {
    perror("socket");
    exit(1);
  }

  // Set SO_REUSEADDR on.
  int on = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0) {
    perror("setsockopt");
    exit(1);
  }

  int flags = fcntl(fd, F_GETFL, 0);

  if (flags == -1) {
    perror("fcntl");
    exit(1);
  }

  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) {
    perror("fcntl");
    exit(1);
  }

  if (bind(fd, (struct sockaddr*)&server, sizeof(server)) != 0) {
    perror("bind");
    exit(1);
  }

  if (listen(fd, 127) != 0) {
    perror("listen");
    exit(1);
  }


  fd_set fds;
  int maxfd = fd;
  struct timeval tv;

  FD_ZERO(&fds);
  FD_SET(fd, &fds);

  while (1) {
    fd_set rfds;

    memcpy(&rfds, &fds, sizeof(fds));

    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    int ready = select(maxfd + 1, &rfds, 0, 0, &tv);

    if (ready == -1) {
      perror("select");
      exit(1);
    }

    if (ready == 0) {

      continue;
    }

    for (int i = 3; i <= maxfd; ++i) {
      if (FD_ISSET(i, &rfds)) {
        if (i == fd) {
          while (1) {
            int nfd = accept(fd, 0, 0);

            if (nfd == -1) {
              if (errno != EWOULDBLOCK) {
                perror("accept");
                exit(1);
              }

              break;
            }

            FD_SET(nfd, &fds);

            if (nfd > maxfd) {
              maxfd = nfd;
            }
          }
        } else {
          char buffer[PACKETSIZE];

          while (1) {
            int n = recv(i, buffer, PACKETSIZE, MSG_DONTWAIT);

            if (n != PACKETSIZE) {
              if (errno == EWOULDBLOCK) {
                break;
              } else if (errno == ECONNRESET) {
                // Close the socket.
                n = 0;
              } else {
                perror("recv");
                exit(1);
              }
            }
            // printf ("recv data %d\n", n);

            if (n == 0) {
              close(i);

              FD_CLR(i, &fds);

              if (i == maxfd) {
                while (!FD_ISSET(maxfd, &fds)) {
                  --maxfd;
                }
              }

              break;
            }
          }
        }
      }
    }
  }
}
