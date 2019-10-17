#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "data.h"
#include "SerialManager.h"
#include <signal.h>

#define BUFFER_SIZE 128

void traduceIsToEduCiaa (char*);
int traduceEduCiaaToIs(char*);
void uartInit (void);
void* toTcpClient (void*);
void bloquearSign(void);
void desbloquearSign(void);
static data_t data; //datos de la conexion y de estado de lineas.


int main()
{
	socklen_t addr_len;
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	char buffer[128];
	//int newfd;
	int n;
	int ret;
	pthread_t toTcpCliThread;//thread hacia el cliente TPC
printf("Inicio");
	// Creamos socket
	int s = socket(PF_INET,SOCK_STREAM, 0);

	// Cargamos datos de IP:PORT del server
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(10000);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(serveraddr.sin_addr.s_addr==INADDR_NONE)
    {
        fprintf(stderr,"ERROR invalid server IP\r\n");
        return 1;
    }

	// Abrimos puerto con bind()
	if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
		close(s);
		perror("listener: bind");
		return 1;
	}

	// Seteamos socket en modo Listening
	if (listen (s, 10) == -1) // backlog=10
  	{
    	perror("error en listen");
    	exit(1);
  	}

	//Inicializo la UART
	//void uartInit (void);
if((serial_open(1,115200))==1)
	{
	printf ("Error al abrir el puerto serie");
	exit (1);	
	}
printf ("Puerto serie abierto");

	while(1)
	{
		// Ejecutamos accept() para recibir conexiones entrantes
		addr_len = sizeof(struct sockaddr_in);
    	if ( (data.conectionFd = accept(s, (struct sockaddr *)&clientaddr, &addr_len)) == -1)
      		{
		perror("Error en accept");
		exit(1);
	   	}
	 	printf  ("server:  conexion desde:  %s\n", inet_ntoa(clientaddr.sin_addr));

		//Creo un thread para la comunicacion hacia el cliente TCP
		bloquearSign();
		ret = pthread_create (&toTcpCliThread, NULL, toTcpClient, &data);
		desbloquearSign();
		if (ret) {	//chequeo que la creacion del thread
			errno = ret;
			perror("pthread_create");
			exit(1);
			}
	
		// Leemos mensaje de cliente
		
		while(1)
		{
			if((n =read(data.conectionFd,buffer,128)) <= 0 )
			{
				perror("Error leyendo mensaje en socket");
				//exit(1);
				break;
			}
			buffer[n]=0;
			printf("Recibi %d bytes:%s\n",n,buffer);
			traduceIsToEduCiaa (buffer);
			serial_send(buffer,strlen(buffer));
			printf("Se envió %s por el puerto serie\n",buffer);
			
				
			
		}
		
		printf ("Error");
		sleep(10);
		pthread_cancel(toTcpCliThread);	//Entonces cierro el thread de transmision hacia el cliente
		printf("El cliente se desconecto ");	
		
		

/*		// Enviamos mensaje a cliente
    	if (write (newfd, "hola", 5) == -1)
    	{
      		perror("Error escribiendo mensaje en socket");
      		exit (1);
    	}
*/

		// Cerramos conexion con cliente
    	close(data.conectionFd);
	}
printf ("saliendo");

	return 0;
}



void traduceIsToEduCiaa (char* buffer)
{
	char l1,l2,l3,l4;
	// Hago el paseo del estado de las lineas en el paquete que recibo de Interface Service
	sscanf(buffer,":STATES%c%c%c%c\n",&l1,&l2,&l3,&l4);
	// Construyo el paquete de salida hacia la UART
//	l1='1';
//l2='0';
//l3='2';
//l4='1';
	sprintf(buffer,">OUTS:%c,%c,%c,%c\r\n",l1,l2,l3,l4);
}


void uartInit (void)
{
int pn, baudrate;
/*
printf ("Ingrese el numero de puerto Serie a utilizar:");
scanf ("%d",&pn);
printf ("Ingrese el baudrate:");
scanf ("%d",&baudrate); */
if((serial_open(1,115200))==1)
	{
	printf ("Error al abrir el puerto serie");
	exit (1);	
	}
printf ("Puerto serie abierto");
}


void bloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    //sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void desbloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    //sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}


void* toTcpClient (void* arg)
{
data_t *data=(data_t*) arg;
//char buffer[BUFFER_SIZE]={":LINE1TG\n"};
char buffer[BUFFER_SIZE];
int i,nr;
int n;
//n=strlen(buffer);
char received;

printf ("Thread1 in");
while(1)
	{
	i=0;
	while(i>=0)
	{
	usleep(10000);
	nr=serial_receive(&received,1);
	if(nr>0)
	{
		if (i+nr>BUFFER_SIZE)
			{
			//Hacer control de desborde
			i=0;
			}	
		if(received!='\n')
		{
			buffer[i]= received;
			i++;
		}
		else
		{
			if(buffer[i-1]=='\r')
				{
				if (traduceEduCiaaToIs (buffer)>0) //traduceEduCiaaToIs devuelve -1 si no pudo hacer la conversion bien
					{
					i=-1;
					n=strlen(buffer);
					}
				else
					i=0; // Si hay error reincio la lectura
				}
		}
	}

	}



		
		// Enviamos mensaje a cliente
	    	if (write (data->conectionFd, buffer, n) < 0)
	    	{
	  		perror("Error escribiendo mensaje en socket");
	      		//exit (1);
			break;

	    	}
		
		sleep(1);
	}
printf ("Thread1 out");
return NULL;
}


int traduceEduCiaaToIs(char* buffer)
{
	char line;
	int ret;
	// Hago el paseo del estado de las lineas en el paquete que recibo de Interface Service
	if (sscanf(buffer,">TOGGLE STATE:%c\r\n",&line)==1)
		{
		// Construyo el paquete de salida hacia la UART
		sprintf(buffer,":LINE%cTG\n",line);
		ret=1;
		}
	else ret=-1;
return (ret);
}


















