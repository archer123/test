#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>       

#define PACKETSIZE 1500

using namespace std;

const int MAX_EPOLL_EVENTS = 1000;

void setFdNonblock(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void err_exit(const char *s){
    printf("error: %s\n",s);
    exit(0);
}

int create_socket(const char *ip, const int port_number)
{
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;           /* ipv4 */
    server_addr.sin_port = htons(port_number);

    if(inet_pton(server_addr.sin_family, ip, &server_addr.sin_addr) == -1){
      err_exit("inet_pton");
    }

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        err_exit("socket");
    }
    int reuse = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        err_exit("setsockopt");
    }
    if(bind(sockfd, (sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        err_exit("bind");
    }
    if(listen(sockfd, 5) == -1){
        err_exit("listen");
    }
    return sockfd;
}

int main(int argc, const char *argv[])
{

    if(argc < 3){
        printf("usage:%s ip port\n", argv[0]);
        exit(0);
    }

    const char * ip = argv[1];
    const int port = atoi(argv[2]);

    int sockfd = create_socket(ip, port);
    printf("success create sockfd %d\n", sockfd);
    setFdNonblock(sockfd);
    
    int epollfd = epoll_create1(0);
    if(epollfd == -1) err_exit("epoll_create1");

    struct epoll_event ev;
    ev.data.fd = sockfd;
    ev.events = EPOLLIN ;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1){
        err_exit("epoll_ctl1");
    }

    struct epoll_event events[MAX_EPOLL_EVENTS] = {0};


    while(1){

        printf("begin wait\n");
        int number = epoll_wait(epollfd, events, MAX_EPOLL_EVENTS, -1);
        printf("end wait\n");
        sleep(1);
        if(number > 0){

            for (int i = 0; i < number; i++)
            {
                int eventfd = events[i].data.fd;
               
                if(eventfd == sockfd){
                    printf("accept new client...\n");
                    struct sockaddr_in client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
                    setFdNonblock(connfd);
                    
                    struct epoll_event ev;
                    ev.data.fd = connfd;
                    ev.events = EPOLLIN;
                    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1){
                        err_exit("epoll_ctl2");
                    }
                    printf("accept new client end.\n");
                }
               
                else{
                    while(1){
                        char buff = -1;
                        int ret = recv(eventfd, &buff, PACKETSIZE, MSG_DONTWAIT);
			if (ret != PACKETSIZE){
			    if(errno == 0){
				break;
			    }
			    else if(errno == EWOULDBLOCK) {
				//printf("1\n");
                		break;
              		    } else if (errno == ECONNRESET) {
				//printf("2\n");
                		ret = 0;
              		    } else {
				//printf("errno %d\n", errno);
                		perror("recv");
                		exit(1);
              		    }	
			}
                        
			if (ret == 0){
                            printf("client close.\n");
                            close(eventfd);
                            epoll_ctl(epollfd, EPOLL_CTL_DEL, eventfd, NULL);
                            break;
                        }
           
                    }
                }
            }
        }
    }
}
