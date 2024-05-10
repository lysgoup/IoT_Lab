CC 			= 	g++ 
CPPFLAGS	= 	-std=c++11 #-Wall 

all: 
	$(CC) $(CPPFLAGS) -I../include Student.cpp StudentClnt.cpp -o ../StudentClnt  -L../lib -lhgunet 
	$(CC) $(CPPFLAGS) -I../include Student.cpp StudentServ.cpp -o ../StudentServ  -L../lib -lhgunet 

clean:
		rm -f ../StudentClnt ../StudentServ
