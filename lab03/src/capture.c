#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <arpa/inet.h> 

#define BUFFER_SIZE 65536

#ifdef DEBUG
    #define debug(fn) fn
#else
    #define debug(fn)
#endif

void print_ethernet_header(char* buffer);
void print_ip_header(char* buffer);
void print_tcp_packet(char* buffer);

int main(int argc, char *argv[]) {
    int raw_socket;
    int num_packets;
    char buffer[BUFFER_SIZE];
    char *interface_name = NULL; 
    struct ifreq ifr;

    if (argc < 2) {
        printf("Usage: %s <Network Interface> \n", argv[0]);
        return -1;
    }

    interface_name = argv[1]; 

    raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket < 0) {
        perror("Failed to create raw socket");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_name, sizeof(ifr.ifr_name) - 1);
    if (setsockopt(raw_socket, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror("Failed to bind raw socket to interface");
        close(raw_socket);
        return -1;
    }

    num_packets = 0;
    while (1) {
        ssize_t length = recv(raw_socket, buffer, BUFFER_SIZE, 0);
        if (length < 0) {
            perror("Failed to receive frame");
            break;
        }

        printf("============================================ \n"); 
        printf("Packet No: %d \n", num_packets);  
        print_ethernet_header(buffer);
        printf("============================================ \n\n"); 

        num_packets++;        
    }

    close(raw_socket);

    return 0;
}


void print_ethernet_header(char* buffer) {
    struct ethhdr *eth = (struct ethhdr*)buffer;

    printf("-------------------------------------------- \n");
    printf("Ethernet Header\n");
    printf("   |-Source MAC Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", 
            eth->h_source[0], eth->h_source[1], eth->h_source[2], 
            eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    printf("   |-Destination MAC Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", 
            eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], 
            eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
    printf("   |-Protocol                : %u \n", (unsigned short)eth->h_proto);
    unsigned short ptype = ntohs((unsigned short)eth->h_proto);

    // Check if the next layer is an IP packet based on the EtherType
    if(ptype == 0x0800){
        char* payload = buffer + sizeof(*eth);
        print_ip_header(payload);
    }
}


void print_ip_header(char* buffer) {
    struct iphdr *ip = (struct iphdr*)buffer;

    printf("-------------------------------------------- \n");
    printf("IP Header\n");
    debug(printf("version: %d\n",ip->version));
    printf("   |-Source IP          : %s \n",inet_ntoa(*(struct in_addr *)&(ip->saddr)));
    printf("   |-Destination IP     : %s \n",inet_ntoa(*(struct in_addr *)&(ip->daddr)));
    printf("   |-Protocol           : %d \n",ip->protocol);
    debug(printf("header len: %d\n",ip->ihl*4));

    if(ip->protocol == 6){
        char* payload = buffer + sizeof(*ip);
        print_tcp_packet(payload);
    }
}


void print_tcp_packet(char* buffer) {
    struct tcphdr *tcp = (struct tcphdr*)buffer;
    printf("-------------------------------------------- \n");
    printf("TCP Packet\n");
    printf("   |-Source Port           : %d \n",ntohs(tcp->source));
    printf("   |-Destination Port      : %d \n",ntohs(tcp->dest));
    printf("   |-Sequence Number       : %d \n",ntohs(tcp->seq));
    printf("   |-Acknowledge Number    : %d \n",ntohs(tcp->ack_seq));
    printf("   |-Flags                 : ");
    if(tcp->urg == 1) printf("URG ");
    if(tcp->ack == 1) printf("ACK ");
    if(tcp->psh == 1) printf("PSH ");
    if(tcp->rst == 1) printf("RST ");
    if(tcp->syn == 1) printf("SYN ");
    if(tcp->fin == 1) printf("FIN ");
    printf("\n");
}
