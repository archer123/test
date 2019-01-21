#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
#include <sys/time.h> 

#define PACKETSIZE 1500

using namespace std;

void err_exit(const char *s){
    printf("error: %s\n",s);
    exit(0);
}

int send_packet(int fd) {
  char buffer[PACKETSIZE];

  gettimeofday((struct timeval*)buffer, 0);
  memset(buffer + sizeof(struct timeval), ' ', (unsigned int)sizeof(buffer) - (unsigned int)sizeof(struct timeval));
  int s = send(fd, buffer, PACKETSIZE, 0);
  if ( s != PACKETSIZE) {
    perror("send");
    exit(1);
  }
  return s;
}


int create_socket(const char *ip, const int port_number)
{
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;           /* ipv4 */
    server_addr.sin_port = htons(port_number);
    if(inet_pton(PF_INET, ip, &server_addr.sin_addr) == -1){
        err_exit("inet_pton");
    }

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        err_exit("socket");
    }
    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        err_exit("connect");
    }

    return sockfd;
}

int main(int argc, const char *argv[]){
    if(argc < 3){
        printf("usage:%s ip port\n", argv[0]);
        exit(0);
    }

    const char * ip = argv[1];
    const int port = atoi(argv[2]);
    time_t next = time(NULL) + 1;

    int sock = create_socket(ip, port);
    int sent = 0;

    char sent_buffer[32];
    while(1){
	
    	time_t now = time(NULL);

    	if (now > next) {
      		snprintf(sent_buffer, sizeof(sent_buffer), "%u mbits/s",  sent * PACKETSIZE * 8 / 1000000 );
      		printf("%s \n", sent_buffer);

     	 	next++;
		sent = 0;
    	}
        send_packet(sock);
	sent++;
    }
    close(sock);
    return 0;
}

