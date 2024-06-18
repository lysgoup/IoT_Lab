all: mqtt_chat_client.c multiple_pub.c multiple_sub.c
	gcc -o pub multiple_pub.c -lmosquitto
	gcc -o sub multiple_sub.c -lmosquitto
	gcc -o chat mqtt_chat_client.c -lmosquitto

chat: mqtt_chat_client.c
	gcc -o chat mqtt_chat_client.c -lmosquitto

clean:
	rm sub pub chat