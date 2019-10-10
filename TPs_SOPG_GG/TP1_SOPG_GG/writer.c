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

volatile sig_atomic_t receivedSig;

//Handler para las señales SIGUSR1 y 2
void hand(int sig)
{
receivedSig = (sig_atomic_t)sig;
//Evaluo cual de las dos señales llamo al handler
if (sig	== SIGUSR1)
	write(1,"SIGUSR1\n",8);
else 
	if (sig	== SIGUSR2)
		write(1,"SIGUSR1\n",8);

}

int main(void)
{
struct sigaction sa;

sa.sa_handler = hand;	//lleno la estructura para cambiar el handler de SIGUSR 1 y 2
sa.sa_flags = 0; 	//SA_RESTART; //0;  //con este flag puedo indicar que hacer cuando ocurre una syscall, se ejecuta el handles y vuelvo a donde interrumpi.
sigemptyset(&sa.sa_mask);

sigaction(SIGUSR1,&sa,NULL);	//le indico cual va a ser el handler de SIGUSR1
sigaction(SIGUSR2,&sa,NULL);	//le indico cual va a ser el handler de SIGUSR2

char outputBuffer[BUFFER_SIZE];
char auxBuffer[BUFFER_SIZE];
uint32_t bytesWrote;
int32_t returnCode;
int32_t fd; //file descriptor de la named fifo	

printf("Writer: Usar comando \"kill -10 %d\" para enviar SIGUSR1\no \"kill -12 %d\" para enviar SIGUSR2 \n\n", getpid(),getpid());

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1 )
    {
        printf("Error creando named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
	printf("Esperando proceso reader...\n");
	if ( (fd = open(FIFO_NAME, O_WRONLY) ) < 0 )
    {
        printf("Error abriendo named fifo file: %d\n", fd);
        exit(1);
    }
    
    /* open syscalls returned without error -> other process attached to named fifo */
	printf("Hay un proceso Reader, escriba algo\n");

    /* Loop forever */
	while (1)
	{
        /* Get some text from console */
		if(fgets(auxBuffer, BUFFER_SIZE, stdin)) //Espero el texto y lo concateno con el string "DATA:"
		{
		strcpy(outputBuffer,"DATA:");	
		strcat(outputBuffer,auxBuffer);
        
        /* Write buffer to named fifo. Strlen - 1 to avoid sending \n char */
			if ((bytesWrote = write(fd, outputBuffer, strlen(outputBuffer)-1)) == -1) //envio el string a la fifo compartida con el reader
      			{
			perror("write");
        		}
        		else
        		{
				
				printf("writer: Se escribieron %d bytes\n", bytesWrote);
		        }
		}
		else//cuando SIGUSR 1 o 2 interrumpe el fgets envio el dato de la señal por la FIFO
		{
			if ((int)receivedSig == SIGUSR1)
				sprintf(outputBuffer,"SIGN:1");
			else
				if ((int)receivedSig == SIGUSR2)
				sprintf(outputBuffer,"SIGN:2");
				else //si fue otra la señal que interrumpio
				{					
				receivedSig=0; 
				outputBuffer[0]=0; //Vacio el buffer para que no se envie nada
				}	
			if ((outputBuffer[0])!=0) //si el buffer no esta vacio envio el string por la FIFO al otro proceso
			{
				if ((bytesWrote = write(fd, outputBuffer, strlen(outputBuffer))) == -1)
      					{
					perror("write");
        				}
        			else
        				{
					printf("writer: wrote %d bytes\n", bytesWrote);
		        		}
			}
				
		}
	}
	return 0;
}
