#include "HGUNet.h"
#include "Student.h"

#define BUF_SIZE 1024

void print_hex(const char *buf, int size);

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	TCPSocketPtr servSocket = SocketUtil::CreateTCPSocket(INET);
	SocketAddress ownAddress(htonl(INADDR_ANY), atoi(argv[1]));
    
	if (servSocket->Bind(ownAddress) != NO_ERROR)
		exit(1);
	
	if (servSocket->Listen(5) != NO_ERROR)
		exit(1);

	for (int i = 0; i < 5; i++) 
	{
		SocketAddress clntAddress;
		TCPSocketPtr clntSocket = servSocket->Accept(clntAddress);
		if (clntSocket == nullptr) 
			continue;
		else 
			LOG("Connected client %d", i+1);

		// Receive a student object 
		Student stu;
		//clntSocket->Receive(reinterpret_cast<char*>(&stu), sizeof(stu));
		stu.Receive(clntSocket);
		printf("Receive student object from client \n");
    	stu.Print();
		printf("\n");
		
		stu.Sleep(); 
		stu.Eat(); 
		stu.Study();
		stu.AddFriend(2222222);
		stu.AddFriend(3333333);
		stu.TestBits();

		// Send a student object 
		printf("\nSend student object to client \n");
		stu.Print();
		//clntSocket->Send(reinterpret_cast<char*>(&stu), sizeof(stu));
		stu.Send(clntSocket);

		clntSocket.reset();
	}

	servSocket.reset();
	
	return 0;
}

void print_hex(const char *buf, int size) 
{
    for (int i = 0; i < size; i++) 
    {
        if (i % 8 == 0) 
            printf("\n");
        printf("0x%02X ", buf[i]);
    }
    printf("\n");
}