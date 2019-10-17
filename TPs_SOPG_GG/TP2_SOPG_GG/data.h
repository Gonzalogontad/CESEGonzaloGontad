#include <pthread.h>

#define LINE_NUM 4
struct data_s{
	int conectionFd;
	int linesState[LINE_NUM]; //estado de las lineas
	pthread_mutex_t mutexData;	
};
typedef struct data_s data_t;
/*
void cd_init(ClientData* clients, int len);
int cd_getFreeIndex(ClientData* clients, int len);
*/
