#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>

#define SEC_WEBSOCKET_KEY "erPKMz5t9vwqkJI+RmHnLw=="
#define CLOSE_CODE 1000
#define BUFFER_SIZE 65536

#ifdef DEBUG
    #define debug(fn) fn
#else
    #define debug(fn)
#endif

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char *argv[]) {
    int sock;
	struct sockaddr_in serv_addr;
    
    // Declare variables 
    char buffer[1024];

    // Check command line arguments.
    if(argc != 4)
    {
        printf("Usage: %s <host> <port> <text>\n"  \
               "Example: \n" \
               "        %s 192.168.10.25 8080 \'Hello, world!\'\n", argv[0], argv[0]); 
        return -1;
    }

    // Connect to host 
    sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	  
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

    // Send WebSocket handshake request
    char *request = "GET %s HTTP/1.1\r\n"
                    "Host: %s:%d\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Key: %s\r\n"
                    "Sec-WebSocket-Version: 13\r\n"
                    "\r\n";                    
    sprintf(buffer, request, argv[1], argv[1], atoi(argv[2]), SEC_WEBSOCKET_KEY);
    send(sock,buffer,strlen(buffer),0);
    
    // Wait for HTTP response from server
    int recv_cnt = recv(sock,buffer,sizeof(buffer),0);
    buffer[recv_cnt] = '\0';
    printf("%s\n",buffer);

    // Verify accept key -> no need to implement 

    // Send WebSocket message
    char *message = argv[3];
    printf("Send message to server: %s\n",message);
    debug(printf("message size: %d\n", strlen(message));)

    size_t payload_len = strlen(message);
    size_t frame_len = 2 + (payload_len < 126 ? 1 : (payload_len < 65536 ? 2 : 8)) + 4 + payload_len;
    char *send_buffer = (char *)malloc(frame_len);
    memset(send_buffer,0,frame_len);

    uint8_t FIN = 0x01;
    char *iterator = send_buffer;
    *(iterator++) = (FIN << 7) | 0x01;

    uint8_t MASK = 0x01;
    *iterator = (MASK << 7);
    if(payload_len<126){
        *(iterator++) |= payload_len;
    }
    else if(payload_len < 65536){
        *(iterator++) |= 126;
        *((uint16_t *)iterator) = htons(payload_len);
        iterator += 2;
    }
    else{
       *(iterator++) |= 127;
        *((uint64_t *)iterator) = htons(payload_len);
        iterator += 8; 
    }
    uint8_t mask_key[4];
    for (int i = 0; i < 4; ++i) {
        mask_key[i] = rand() % 256;
        *(iterator++) = mask_key[i];
    }
    
    uint8_t *mask_temp = (uint8_t *)(&mask_key);
    for(int i=0;i<payload_len;i++){
        message[i] = mask_temp[i%4] ^ message[i];
    }
    message[payload_len] = '\0';

    strcpy(iterator,message);
    // send(sock,send_buffer,strlen(send_buffer),0);
    send(sock,send_buffer,frame_len,0);
    // debug(printf("check: %d\n",strlen(send_buffer)));
    debug(for(int i=0;i<7;i++){
        printf("%x\n",send_buffer[i]);
    });
    free(send_buffer);
    // Wait for echo response message from server
    memset(buffer,sizeof(buffer),0);
    recv_cnt = recv(sock,buffer,sizeof(buffer),0);
    debug(printf("response: %d\n",recv_cnt);)
    iterator = buffer;
    iterator++;
    MASK = *iterator & 0x80;
    debug(printf("mask: %d\n",MASK);)
    payload_len = *(iterator++) & ~0x80;
    debug(printf("recv_cnt: %d\n",payload_len);)
    if(payload_len == 126){
        payload_len = ntohs(*((uint16_t *)iterator));
        iterator += 2;
    }
    else if(payload_len == 127){
        iterator += 8;
    }
    printf("received payload_len: %d\n",payload_len);
    char *recv_message = (char *)malloc(payload_len+1);
    memset(recv_message,0,payload_len+1);
    strncpy(recv_message,iterator,payload_len);
    recv_message[payload_len] = '\0';
    printf("Receive message from server: %s\n",recv_message);
    free(recv_message);

    // Send close frame: Close code = 1000 
    // Close code must be converted to network byte order via htons() 
    payload_len = 2;
    size_t frame_size = 8;
    char *close_frame = (char *)malloc(frame_size);
    iterator = close_frame;
    *(iterator++) = 0x88;
    *(iterator++) = 0x80 | (char)payload_len;
    for (int i = 0; i < 4; ++i) {
        mask_key[i] = rand() % 256;
        *(iterator++) = mask_key[i];
    }
    uint16_t payload_data = htons(CLOSE_CODE);
    uint8_t* byte_ptr = (uint8_t *)&payload_data;
    for (size_t i = 0; i < payload_len; ++i) {
        *(iterator++) = byte_ptr[i] ^ mask_key[i % 4];
    }
    int send_cnt = send(sock,close_frame,frame_size,0);
    debug(printf("send_cnt: %d, frame_size: %d\n",send_cnt, frame_size);)
    debug(for(int i=0;i<frame_size;i++){
        printf("%x\n",close_frame[i]);
    });
    free(close_frame);
    
    // Receive close response from server 
    memset(buffer,sizeof(buffer),0);
    recv_cnt = recv(sock,buffer,sizeof(buffer),0); 
    debug(printf("recv_cnt: %d\n",recv_cnt);)

    // Close socket descriptor
    close(sock);
    return 0;
}
