#include "toTcpClientThread.h"
#include <pthread.h>

//Thread que se encarga de la comunicacion desde la EDU-CIAA hacia el socket TCP.
void* toTcpClient (void* arg)
{
	int *conectionFd=(int*) arg;
	char buffer[BUFFER_SIZE];
	int i,nr;	//Contadores parciales 
	int n;		//Contador total de caracteres en el buffer
	char received; 	//Caracter recivido


	while(1)
	{
		i=0;
		
		while(i>=0)//Si hay un mensaje para enviar salgo con i<0
		{
				//Espero 10 ms antes de recibir otro caracter para no llevar el proceso al 100%
				usleep(10000);
				nr=serial_receive(&received,1); 
				// Cuando llega un caracter lo proceso
				if(nr>0)
				{
						//Control de desborde de buffer
					if (i+nr>(BUFFER_SIZE-1))  //Dejo un lugar para el fin de cadena
						{
						 i=0;
						}
						//Veo si llegue al final del mensaje
					if(received!='\n') 
					{
						buffer[i]= received;
						i++;
					}
					else
					{
						if(buffer[i-1]=='\r')
							{
							//Cuando llego al final del mensaje
							buffer[i+1]=0;//Agrego el fin de cadena
							if (traduceEduCiaaToIs (buffer)>0) //traduceEduCiaaToIs devuelve -1 si no pudo hacer la conversion bien
								{
								i=-1; //Si la conversion fue correcta salgo del while para enviar el mensaje
								n=strlen(buffer);
								}
							else
								i=0; // Si hay error reincio la lectura
							}
					}
				}

		}

		// Envio el mensaje a cliente
		if (write (*conectionFd, buffer, n) < 0)
		{
			perror("Error escribiendo mensaje en socket");
			break;
		}
		sleep(1);

	}

	return NULL;
}
