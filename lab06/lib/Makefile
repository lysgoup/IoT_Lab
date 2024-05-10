CC 			= 	g++ 
LIB 		= 	libhgunet.a 
OBJS		=	StringUtils.o SocketAddress.o SocketUtil.o TCPSocket.o MemoryStream.o 
CPPFLAGS	= 	-std=c++11 #-Wall 

$(LIB): $(OBJS)
		$(AR) rv $@ $(OBJS) 

%.o: %.cpp
		$(CC) -c $(CPPFLAGS) -I../include $< -o $@

clean:
		rm -f $(OBJS) $(LIB)

