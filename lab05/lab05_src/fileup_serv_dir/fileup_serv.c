#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 40
#define EPOLL_SIZE 50
#define MAX_USER 100
void error_handling(char *buf);

typedef struct clntfile{
	int flag;
	int filename_len;
	char *filename;
	FILE *fp;
}clntfile;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t adr_sz;
	int str_len, i;
	char buf[BUF_SIZE];

	struct epoll_event *ep_events;
	struct epoll_event event;
	int epfd, event_cnt;

	clntfile clnt_array[MAX_USER];
	memset(clnt_array,0,sizeof(struct clntfile)*MAX_USER);

	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

	epfd=epoll_create(EPOLL_SIZE);
	ep_events=malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

	event.events=EPOLLIN;
	event.data.fd=serv_sock;	
	epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

	while(1)
	{
		event_cnt=epoll_wait(epfd, ep_events, EPOLL_SIZE,-1);
		if(event_cnt==-1)
		{
			puts("epoll_wait() error");
			break;
		}

		puts("return epoll_wait");
		for(i=0; i<event_cnt; i++)
		{
			if(ep_events[i].data.fd==serv_sock)
			{
				adr_sz=sizeof(clnt_adr);
				clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
				event.events=EPOLLIN;
				event.data.fd=clnt_sock;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
				printf("connected client: %d \n", clnt_sock);
				if(clnt_array[clnt_sock].filename != NULL){
					free(clnt_array[clnt_sock].filename);
				}
				clnt_array[clnt_sock].flag = 1;
			}
			else
			{
        int index = ep_events[i].data.fd;
				if(clnt_array[index].flag){
					read(ep_events[i].data.fd, &clnt_array[index].filename_len, sizeof(int));
					printf("filename len: %d\n",clnt_array[index].filename_len);
					clnt_array[index].filename = (char *)malloc(clnt_array[index].filename_len);
					read(ep_events[i].data.fd, clnt_array[index].filename, clnt_array[index].filename_len);
					printf("filename: %s\n",clnt_array[index].filename);
					clnt_array[index].fp = fopen(clnt_array[index].filename,"wb");
					clnt_array[index].flag = 0;
				}
				else{
					str_len=read(ep_events[i].data.fd, buf, BUF_SIZE);
					if(str_len==0)    // close request!
					{
						epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
						close(ep_events[i].data.fd);
						printf("closed client: %d\n", ep_events[i].data.fd);
						fclose(clnt_array[index].fp);
						free(clnt_array[index].filename);
						memset(&clnt_array[index],0,sizeof(struct clntfile));
					}
					else
					{
						fwrite(buf, 1, str_len, clnt_array[index].fp);
					}
				}
			}
		}
	}
	close(serv_sock);
	close(epfd);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}