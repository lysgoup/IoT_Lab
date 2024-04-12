PORT = 7777

ifdef D
	DEBUG = -DDEBUG
endif

all: tcp_client.c tcp_server.c
	gcc -o tcp_client tcp_client_raw.c $(DEBUG)
	gcc -o tcp_server tcp_server.c

iptables:
	sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP

run_serv:
	./tcp_server $(PORT)

run_cli:
	sudo ./tcp_client 127.0.0.1 127.0.0.1 $(PORT)