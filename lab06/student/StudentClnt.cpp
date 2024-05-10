#include "HGUNet.h"
#include "Student.h"

void send_student(TCPSocketPtr socket, const Student* inStudent);
void receive_student(TCPSocketPtr socket, Student* outStudent);
void print_hex(const char *buf, int size);

int main(int argc, char **argv) 
{
    if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
    // Create student object
    Student stu(1111111, "Seoeun");
    TCPSocketPtr socket = SocketUtil::CreateTCPSocket(INET);
	SocketAddress servAddress(inet_addr(argv[1]), atoi(argv[2]));

	if (socket->Connect(servAddress) != NO_ERROR)
		exit(1);
        
    // Send a student object 
    printf("Send student object to server \n");
    stu.Print();
    //socket->Send(reinterpret_cast<char*>(&stu), sizeof(stu));
    stu.Send(socket);  

    // Receive a student object 
    //socket->Receive(reinterpret_cast<char*>(&stu), sizeof(stu));
    stu.Receive(socket); 
    printf("\nReceive student object from server \n");
    stu.Print();

	socket.reset();

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