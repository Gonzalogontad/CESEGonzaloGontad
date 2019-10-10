#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 300



int main(void)
{

	
uint8_t inputBuffer[BUFFER_SIZE];
int32_t bytesRead, returnCode, fd;
int logFD; 
int signFD;
int bytesWrote;

//Abro los archivos Log.txt y Sign.txt y si no existen los crea. Apunto al final del archivo
if((logFD = open("Log.txt",O_RDWR|O_APPEND|O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO))==-1)//Le doy permiso a todos los niveles de usuario
	{ 	
	perror("Cannot open output file Log.txt\n"); 
	exit(1);
	}
if ((signFD = open("Sign.txt",O_RDWR|O_APPEND|O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO))==-1)//Le doy permiso a todos los niveles de usuario
	{
	perror("Cannot open output file Log.txt\n"); 
	exit(1);  
	}
 
    /* Create named fifo. -1 means already exists so no action if already exists */
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1  )
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }
    
    /* Open named fifo. Blocks until other process opens it */
	printf("Esperando poceso writer...\n");
	if ( (fd = open(FIFO_NAME, O_RDONLY) ) < 0 )
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }
    
    /* open syscalls returned without error -> other process attached to named fifo */
	printf("Hay un writer\n");

    /* Loop until read syscall returns a value <= 0 */
	do
	{
        /* read data into local buffer */
		if ((bytesRead = read(fd, inputBuffer, BUFFER_SIZE)) == -1)
        	{
			perror("read");
        	}
        else
		{
		inputBuffer[bytesRead] = '\0';
		strcat(inputBuffer,"\n");
		if ( strncmp(inputBuffer,"DATA:", sizeof("DATA:")-1)==0)  //si llegó un string de DATA
			{
			if ((bytesWrote = write(logFD, inputBuffer, strlen(inputBuffer))) == -1) //Escribo en el archivo log.txt
      			{
				perror("write");
        		}
       			else
       				{
				printf("reader: Llegaron %d bytes de DATOS: %s \nSe guardaron en Log.txt %d bytes \n \n", bytesRead, inputBuffer,bytesWrote);
				}
			}
		else
			if ( strncmp(inputBuffer,"SIGN:", sizeof("SIGN:")-1)==0)  //si llegó un string de SIGN
			{
				if ((bytesWrote = write(signFD, inputBuffer, strlen(inputBuffer))) == -1) //Escribo en el archivo sign.txt
      				{
				perror("write");
        			}
        			else
        				{				
					printf("reader: Llego la señal: %s \nSe guardaron en Sign.txt %d bytes \n \n", inputBuffer,bytesWrote);
					}
			}
		}
	}
	while (bytesRead > 0);
close (signFD); //Cierro los dos archivos
close(logFD);	
printf("saliendo");

	return 0;
}
