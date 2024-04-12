#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#ifdef DEBUG
	#define debug(fn) fn
#else
	#define debug(fn)
#endif

#define MSG_SIZE 1024
#define BUF_SIZE 65536

// Before run this code, execute the command below 
// $ sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP
void make_tcp_header(char *packet, int type, struct sockaddr_in saddr, struct sockaddr_in daddr, uint32_t seq_num, uint32_t ack_num, char * buffer, size_t data_len);
void make_ip_header(char *packet, struct sockaddr_in saddr, struct sockaddr_in daddr, uint16_t length);
unsigned short checksum(uint16_t *packet ,size_t len);
void make_pseudo_header(void* temp_c, uint32_t saddr, uint32_t daddr, uint8_t protocol, size_t tcp_len);

typedef enum _TCP_TYPE{
	SYN,
	ACK,
	DATA
}TCP_TYPE;

// TODO: pseudo header needed for tcp header checksum calculation
struct pseudo_header
{
	uint32_t source_address;
	uint32_t dest_address;
	uint8_t placeholder;
	uint8_t protocol;
	uint16_t tcp_length;
};

// TODO: Define checksum function which returns unsigned short value 
unsigned short checksum(uint16_t *buf ,size_t len)
{
	unsigned int sum = 0;
	unsigned short checksum = 0;
	while(len>1){
		sum += *buf;
		buf++;
		len -= 2;
	}
	if(len == 1){
		unsigned short temp = 0;
		*((uint8_t *)&temp)=*(uint8_t*)buf;
		sum += temp;
		len -= 1;
	}
	sum = (sum & 0xffff) + (sum >> 16);
	checksum = ~sum;
	return checksum;
}

void make_tcp_header(char *packet, int type, struct sockaddr_in saddr, struct sockaddr_in daddr, uint32_t seq_num, uint32_t ack_num, char * buffer, size_t data_len)
{
	struct tcphdr *tcp = (struct tcphdr *)packet;
	tcp->source = saddr.sin_port;
	tcp->dest = daddr.sin_port;
	tcp->seq = htonl(seq_num);
	tcp->ack_seq = htonl(ack_num);
	tcp->doff = 5;
	tcp->res1 = 0;
	tcp->res2 = tcp->urg = tcp->rst = tcp->fin = 0;
	if(type==SYN){
		tcp->syn = 1;
		tcp->ack = 0;
		tcp->psh = 0;
	}
	else if(type==ACK){
		tcp->syn = 0;
		tcp->ack = 1;
		tcp->psh = 0;
	}
	else if(type==DATA){
		tcp->syn = 0;
		tcp->ack = 1;
		tcp->psh = 1;
	}
	tcp->window = htons(16384);
	tcp->check = 0;
	tcp->urg_ptr = 0;

	unsigned char *temp_c = (unsigned char *)malloc(sizeof(struct pseudo_header)+sizeof(struct tcphdr)+data_len);
	make_pseudo_header(temp_c, saddr.sin_addr.s_addr, daddr.sin_addr.s_addr, 6, sizeof(struct tcphdr)+data_len);
	memcpy(temp_c+sizeof(struct pseudo_header),tcp,sizeof(struct tcphdr));
	memcpy(temp_c+sizeof(struct pseudo_header)+sizeof(struct tcphdr),buffer,data_len);
	debug(printf("pseudo header tcp len: %hi\n",ntohs(((struct pseudo_header *)temp_c)->tcp_length));)

	tcp->check = checksum((uint16_t *)temp_c, sizeof(struct pseudo_header)+sizeof(struct tcphdr)+data_len);
	free(temp_c);
	debug(printf("tcp source port: %d\n",ntohs(tcp->source));)
	debug(printf("tcp dest port: %d\n",ntohs(tcp->dest));)
	debug(printf("tcp seq num: %u\n",ntohl(tcp->seq));)
	debug(printf("tcp ack num: %u\n",ntohl(tcp->ack_seq));)
	debug(printf("tcp checksum: %x\n",ntohs(tcp->check));)
}

void make_ip_header(char *packet, struct sockaddr_in saddr, struct sockaddr_in daddr, uint16_t length)
{
	struct iphdr *ip = (struct iphdr *)packet;
	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = htons(length);
	ip->id = htons(rand()%65535);
	ip->frag_off = htons(1 << 14);
	ip->ttl = 60;
	ip->protocol = 6; //tcp
	ip->check = 0;
	ip->saddr = saddr.sin_addr.s_addr;
	ip->daddr = daddr.sin_addr.s_addr;
	ip->check = checksum((uint16_t *)ip, sizeof(struct iphdr));
	debug(printf("ip checksum: %x\n",ip->check);)
}

void make_pseudo_header(void* temp_c, uint32_t saddr, uint32_t daddr, uint8_t protocol, size_t tcp_len)
{
	struct pseudo_header *temp = (struct pseudo_header *)temp_c;
	temp->source_address = saddr;
	temp->dest_address = daddr;
	temp->placeholder = 0;
	temp->protocol = protocol;
	temp->tcp_length = htons(tcp_len);
}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		printf("Usage: %s <Source IP> <Destination IP> <Destination Port>\n", argv[0]);
		return 1;
	}

	srand(time(NULL));

	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (sock == -1)
	{
		perror("socket");
        exit(EXIT_FAILURE);
	}

	// Source IP
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(rand() % 65535); // random client port
	if (inet_pton(AF_INET, argv[1], &saddr.sin_addr) != 1)
	{
		perror("Source IP configuration failed\n");
		exit(EXIT_FAILURE);
	}

	// Destination IP and Port 
	struct sockaddr_in daddr;
	daddr.sin_family = AF_INET;
	daddr.sin_port = htons(atoi(argv[3]));
	if (inet_pton(AF_INET, argv[2], &daddr.sin_addr) != 1)
	{
		perror("Destination IP and Port configuration failed");
		exit(EXIT_FAILURE);
	}

	// Tell the kernel that headers are included in the packet
	int one = 1;
	const int *val = &one;
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) == -1)
	{
		perror("setsockopt(IP_HDRINCL, 1)");
		exit(EXIT_FAILURE);
	}

	// TCP Three-way Handshaking 
	srand(time(NULL));
	uint32_t seq_num = rand();
	uint32_t ack_num = 0;
	debug(printf("Initial seq_num : %u\n",seq_num));

	int tcp_type;
	const size_t tcp_header_len = sizeof(struct tcphdr);
	const size_t ip_header_len = sizeof(struct iphdr);
	int data_len = 0;
	
	// Step 1. Send SYN (no need to use TCP options)
	debug(printf("------------SYN------------\n");)
	tcp_type = SYN;
	unsigned char *SYN_packet = (unsigned char *)malloc(tcp_header_len+ip_header_len);
	make_tcp_header(SYN_packet+ip_header_len, tcp_type, saddr, daddr, seq_num, ack_num, NULL, data_len);
	make_ip_header(SYN_packet, saddr, daddr, tcp_header_len+ip_header_len);
	int check = sendto(sock, SYN_packet, tcp_header_len+ip_header_len, 0, (struct sockaddr *)&daddr, sizeof(daddr));
	free(SYN_packet);

	// Step 2. Receive SYN-ACK
	debug(printf("----------SYN/ACK----------\n");)
	char buffer[BUF_SIZE];
	ssize_t received;
	struct iphdr *synack_ip = (struct iphdr *)buffer;
	struct tcphdr *synack_tcp = (struct tcphdr *)(buffer+sizeof(struct iphdr));	
	while((received = recv(sock,buffer,BUF_SIZE,0))>0){
		if(synack_ip->daddr==saddr.sin_addr.s_addr && synack_tcp->dest==saddr.sin_port && synack_tcp->ack==1 && synack_tcp->syn==1) break;
	}
	if(received<=0){
		fprintf(stderr,"Could not receive SYNACK\n");
		close(sock);
		exit(EXIT_FAILURE);
	}
	seq_num = ntohl(synack_tcp->ack_seq);
	ack_num = ntohl(synack_tcp->seq)+1;

	debug(printf("SYN-ACK length: %d\n",received);)
	debug(printf("SYN: %d\n",synack_tcp->syn);)
	debug(printf("ACK: %d\n",synack_tcp->ack);)
	debug(printf("SEQ_num: %u\n",ntohl(synack_tcp->seq));)
	debug(printf("ACK_num: %u\n",ntohl(synack_tcp->ack_seq));)

	// Step 3. Send ACK 
	debug(printf("------------ACK------------\n");)
	tcp_type = ACK;
	unsigned char *ACK_packet = (unsigned char *)malloc(tcp_header_len+ip_header_len);
	make_tcp_header(ACK_packet+ip_header_len, tcp_type, saddr, daddr, seq_num, ack_num, NULL, data_len);
	make_ip_header(ACK_packet, saddr, daddr, tcp_header_len+ip_header_len);
	check = sendto(sock, ACK_packet, tcp_header_len+ip_header_len, 0, (struct sockaddr *)&daddr, sizeof(daddr));
	free(ACK_packet);
	
	// Data transfer 
	char message[MSG_SIZE];
	while (1) 
	{
		debug(printf("------------DATA------------\n");)
		fputs("Input message(Q to quit): ", stdout);
		fgets(message, MSG_SIZE, stdin);
		
		if (!strcmp(message,"q\n") || !strcmp(message,"Q\n"))
			break;

		// Step 4. Send an application message (with PSH and ACK flag)! 
		tcp_type = DATA;
		data_len = strlen(message);
		debug(printf("data: %s\n",message);)
		debug(printf("data len: %d\n",data_len);)
		unsigned char *DATA_packet = (unsigned char *)malloc(tcp_header_len+ip_header_len+data_len);
		make_tcp_header(DATA_packet+ip_header_len, tcp_type, saddr, daddr, seq_num, ack_num, message, data_len);
		make_ip_header(DATA_packet, saddr, daddr, tcp_header_len+ip_header_len+data_len);
		memcpy(DATA_packet+ip_header_len+tcp_header_len,message,data_len);
		check = sendto(sock, DATA_packet, tcp_header_len+ip_header_len+data_len, 0, (struct sockaddr *)&daddr, sizeof(daddr));
		free(DATA_packet);
		
		// Step 5. Receive ACK 
		debug(printf("------------ACK------------\n");)
		char buffer[BUF_SIZE];
		struct iphdr *ack_ip = (struct iphdr *)buffer;
		struct tcphdr *ack_tcp = (struct tcphdr *)(buffer+sizeof(struct iphdr));	
		while((received = recv(sock,buffer,BUF_SIZE,0))>0){
			if(ack_ip->daddr==saddr.sin_addr.s_addr && ack_tcp->dest==saddr.sin_port && ack_tcp->ack==1 && ack_tcp->syn==0) break;
		}
		if(received<=0){
			fprintf(stderr,"Could not receive ACK\n");
			close(sock);
			exit(EXIT_FAILURE);
		}
		seq_num = ntohl(ack_tcp->ack_seq);
		debug(printf("ACK length: %d\n",received));
		debug(printf("SYN: %d\n",ack_tcp->syn);)
		debug(printf("ACK: %d\n",ack_tcp->ack);)
		debug(printf("SEQ_num: %u\n",ntohl(ack_tcp->seq));)
		debug(printf("ACK_num: %u\n",ntohl(ack_tcp->ack_seq));)	
	}

	close(sock);
	return 0;
}