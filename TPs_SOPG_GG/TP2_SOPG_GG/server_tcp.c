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
#include "SerialManager.h"
#include <signal.h>
#include "transCode.h"
#include "toTcpClientThread.h"
#include <pthread.h>

#define BUFFER_SIZE 128

void uartInit (void);		//Inicializacion de la uart
void bloquearSign(void);	//Bloquear señales
void desbloquearSign(void);  	//Desbloquear señales 
void hand(int );		//Handler de las señales SIGINT y SIGTERM

volatile sig_atomic_t receivedSig=0; //Señal Recibida



int main()
{
	// Captura de las señales SIGINT y SIGTERM
	struct sigaction sa;
	sa.sa_handler = hand;	//lleno la estructura para cambiar el handler de SIGUSR 1 y 2
	sa.sa_flags = 0; 	//SA_RESTART; //0;  //con este flag puedo indicar que hacer cuando ocurre una syscall, se ejecuta el handles y vuelvo a donde interrumpi.
	sigemptyset(&sa.sa_mask);

	if(sigaction(SIGINT,&sa,NULL)<0)	//le indico cual va a ser el handler de SIGINT
	{
		perror ("sigaction");
		return 1;
	}
	if(sigaction(SIGTERM,&sa,NULL)<0)	//le indico cual va a ser el handler de SIGTERM
	{
		perror ("sigaction");
		return 1;
	}

	// Socket TCP

    int conectionFd;	//File descriptor de la conexion al socket
    socklen_t addr_len;
    struct sockaddr_in clientaddr; //Direccion del cliente
    struct sockaddr_in serveraddr; //Direccion del servidor (local)
    char buffer[128]; //Buffer de recepcion por TCP
    int n;
    int ret;
    pthread_t toTcpCliThread;	//Thread hacia el cliente TPC

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
        fprintf(stderr,"\nERROR invalid server IP\r\n");
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

    //Obtengo el id del proceso y lo muestro
	printf("\nWriter: Usar comando \"kill -2 %d\" para enviar SIGINT\no \"kill -15 %d\" para enviar SIGTERM \n\n", getpid(),getpid());

    //Inicializo la UART
	uartInit();

    while(1)
    {
		if (receivedSig	== SIGINT||receivedSig	== SIGTERM)
		{
				// Cierro el thread hacia el cliente TCP
			//pthread_cancel(toTcpCliThread);	//No es necesario el pthread_cancel porque ya se cerro en la linea 150
				// Cierro conexion con cliente
			//close(conectionFd); //No es necesario el close porque ya se cerro en la linea 152
				// Cierro el puerto serie
			serial_close();
			printf("\nSaliendo por señal SIGINT o SIGTERM\n");
			exit(1);
		}
	    	
			// Ejecutamos accept() para recibir conexiones entrantes
	    addr_len = sizeof(struct sockaddr_in);
	    if ( (conectionFd = accept(s, (struct sockaddr *)&clientaddr, &addr_len)) == -1)
	    {
			perror("Error en accept");
			exit(1);
		}
	    printf  ("server:  conexion desde:  %s\n", inet_ntoa(clientaddr.sin_addr));

	    //Creo un thread para la comunicacion hacia el cliente TCP
	    bloquearSign();
	    ret = pthread_create (&toTcpCliThread, NULL, toTcpClient, &conectionFd);
	    desbloquearSign();
	    if (ret) {  //chequeo que la creacion del thread
	        errno = ret;
	        perror("pthread_create");
	        exit(1);
	        }
	
	    // Leemos mensaje de cliente
	    
	    while(1)
	    {
			//Llego SIGINT O SIGTERM salgo del lazo para cerrar el programa
			if (receivedSig	== SIGINT||receivedSig	== SIGTERM)
				break;

			//Espero a que llegue un mensaje 
		    if((n =read(conectionFd,buffer,128)) <= 0 )
		    {
		        perror("Error leyendo mensaje en socket");	// Si hay error, el cliente se desconecta o interrumpe una señal
		        break;										// salgo del lazo de recepcion
		    }
		    buffer[n]=0; //Agrego fin de cadena
		    printf("\nRecibi %d bytes:%s\n",n,buffer); //Imprimo lo que recibi por TCP
		    
				//Convierto el mensaje al formato de la eduCiaa y lo envio por puerto serie
		    traduceIsToEduCiaa (buffer);
		    serial_send(buffer,strlen(buffer));	
		    printf("\nSe envio %s por el puerto serie\n",buffer);
		}
			// Si llego aca es porque hubo error de lectura, se desconecto el cliente o llego una señal
		pthread_cancel(toTcpCliThread); // Entonces cierro el thread de transmision hacia el cliente
		    // Cerramos conexion con cliente
	    close(conectionFd);
		printf("\nCliente desconectado\n"); 
    }
	//No deberia llegar nunca aca
	// Cierro el thread hacia el cliente TCP
	pthread_cancel(toTcpCliThread);	
	// Cierro conexion con cliente
	close(conectionFd);
	// Cierro el puerto serie
	serial_close();

	printf ("\nSaliendo\n");

    return 0;
}




//Inicializacion de la UART
void uartInit (void)
{
	int pn, baudrate;

	//Leo N° de puerto y baudrate
	printf ("\nIngrese el numero de puerto Serie a utilizar(Si es TTY1 ingrese 1):\n");
	scanf ("%d",&pn);
	printf ("\nIngrese el baudrate (115200 para el TP2):\n");
	scanf ("%d",&baudrate); 
	if((serial_open(pn,baudrate))==1)
		{
		printf ("\nError al abrir el puerto serie\n");
		exit (1);   
		}
	printf ("\nPuerto serie abierto\n");
}

//Bloquear señales SIGINT y SIGTERM
void bloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

//Desbloquea las señales SIGINT y SIGTERM
void desbloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}




//Handler para las señales SIGINT y SIGTERM
void hand(int sig)
{
	receivedSig = (sig_atomic_t)sig;
	//Evaluo cual de las dos señales llamo al handler
	if (sig	== SIGINT)
		{
		write(1,"\nLlego SIGINT\n",14);
		  	}
	if (sig	== SIGTERM)
		{
		write(1,"\nLlego SIGTERM\n",15);
		}
}






















