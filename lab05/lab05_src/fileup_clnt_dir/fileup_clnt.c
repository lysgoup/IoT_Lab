#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *buf);

int main(int argc, char *argv[])
{
	int sock;
	char buf[BUF_SIZE];
	int str_len, recv_len, recv_cnt;
	struct sockaddr_in serv_adr;
	FILE * fp;

	if (argc != 4) {
		printf("Usage : %s <IP> <port> <file name>\n", argv[0]);
		exit(1);
	}
	
	sock = socket(PF_INET, SOCK_STREAM, 0);   
	if (sock == -1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));
	
	if (connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("connect() error!");
	else
		puts("Connected...........");
	
	fp = fopen(argv[3],"rb");
	if(fp==NULL){
		perror("fopen");
		return 0;
	}
	int filename_len = strlen(argv[3]);
	printf("- filename: %s\n",argv[3]);
	printf("- filename len: %d\n",filename_len);
	write(sock,&filename_len,sizeof(int));
	str_len = write(sock, argv[3], strlen(argv[3]));
	
	int read_cnt;
	while (1) {
		read_cnt = fread((void *)buf, 1, BUF_SIZE, fp);
		if (read_cnt < BUF_SIZE) {
			write(sock, buf, read_cnt);
			// printf("sending: %s\n",buf);
			break;
		}
		write(sock, buf, BUF_SIZE);
		// printf("sending: %s\n",buf);
	}
	fclose(fp);
	close(sock);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}
