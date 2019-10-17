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
//#include <data.h>

static data_t data; //datos de la conexion y de estado de lineas.

void traduceIsToEduCiaa (char*);
void uartInit (void)

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
	void uartInit (void)

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
		if (!ret) {	//chequeo que la creacion del thread
			errno = ret;
			perror("pthread_create");
			exit(1);
			}
}
		// Leemos mensaje de cliente
		while( (n =read(data.conectionFd,buffer,128)) != -1 )
		{
			if (n=0)	//si llego un cero significa que el cliente se desconecto
				{
				pthread_cancel(toTcpCliThread);	//Entonces cierro el thread de transmision hacia el cliente
				printf("El cliente se desconecto ");				
				break;
				}
			else
			buffer[n]=0;
			printf("Recibi %d bytes:%s\n",n,buffer);
			traduceIsToEduCiaa (buffer);
			serial_send(buffer,strlen(buffer));
			printf("Se envió %s por el puerto serie\n",buffer);
		}
		if(n == -1)
		{
			perror("Error leyendo mensaje en socket");
			exit(1);
		}

		

/*		// Enviamos mensaje a cliente
    	if (write (newfd, "hola", 5) == -1)
    	{
      		perror("Error escribiendo mensaje en socket");
      		exit (1);
    	}
*/

		// Cerramos conexion con cliente
    	close(newfd);
	}

	return 0;
}



void traduceIsToEduCiaa (char* buffer)
{
	char l1,l2,l3,l4;
	// Hago el paseo del estado de las lineas en el paquete que recibo de Interface Service
	sscanf(buffer,”:STATES%c%c%c%c\n”,&l1,&l2,&l3,&l4);
	// Construyo el paquete de salida hacia la UART
	sprintf(buffer,”>OUTS:%c,%c,%c,%c\r\n”,&l1,&l2,&l3,&l4);
}


void uartInit (void)
{
int pn, baudrate;

printf ("Ingrese el numero de puerto Serie a utilizar:");
scanf ("%d",&pn);
printf ("Ingrese el baudrate:");
scanf ("%d",&baudrate);
if((serial_open(pn,baudrate))==1)
	{
	printf ("Error al abrir el puerto serie");
	exit (1);	
	}
}























