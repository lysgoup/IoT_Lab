#include <stdio.h>
#include <string.h>
#include <mosquitto.h>
#include <pthread.h>
#include <stdlib.h>

#define MQTT_HOST 	"test.mosquitto.org"
#define MQTT_PORT   1883
#define MQTT_TOPIC  "chat/yunseo"

static void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
   // TODO
   printf("Connected to MQTT broker\n");
    if(rc != 0){
        mosquitto_disconnect(mosq);
    }
}

static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    // TODO
    printf("%s\n", (char *)msg->payload);
}


void publish_message(struct mosquitto *mosq, char *name)
{
	int rc;
	char payload[20];

	fgets(payload,sizeof(payload),stdin);
	char *data = (char *)malloc(strlen(payload)+strlen(name)+3);
	snprintf(data,strlen(payload)+strlen(name)+4,"[%s]: %s",name,payload);
	rc = mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(data), data, 2, false);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
	}
}

void *sub_topic(){
	struct mosquitto *mosq;
	int rc;

	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, NULL);
	if(mosq == NULL){
		fprintf(stderr, "Error: Out of memory.\n");
		pthread_exit(NULL);
	}

	mosquitto_message_callback_set(mosq, on_message);

	rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		pthread_exit(NULL);
	}

	rc = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 1);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
		/* We might as well disconnect if we were unable to subscribe */
		mosquitto_disconnect(mosq);
	}

	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_lib_cleanup();
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
			printf("Usage: %s <ClientID> \n", argv[0]);
			exit(1);
	}

	pthread_t sub_tid;
	int res = pthread_create(&sub_tid,NULL,sub_topic,NULL);
	if(res != 0){
			printf("Thread creation error\n");
			exit(1);
	}

	struct mosquitto *mosq;
	int rc;

	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, NULL);
	if(mosq == NULL){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	mosquitto_connect_callback_set(mosq, on_connect);
	rc = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	/* Run the network loop in a background thread, this call returns quickly. */
	rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	while(1){
		publish_message(mosq,argv[1]);
	}

	mosquitto_lib_cleanup();
    
  return 0;
}
