/*
 * queuesAndSem.c
 *
 *  Created on: 6 ago. 2019
 *      Author: gonzalo
 */

#include "sapi.h"
#include "queuesAndSem.h"


void queuesAndSemInit(void)
{
	uint8_t i;
	uint8_t j=1; //variable auxiliar para chequear la correcta creacion de semaforos y colas

	/*----Creacion de Colas----*/

	UARTOutQueue=xQueueCreate( 100, sizeof(char) ); //creo la cola de salida para la UART de 100 char

	UARTInQueue=xQueueCreate( 200, sizeof(char) ); //creo la cola de entrada para la UART de 200 char

	for (i=0;i<ADCNUM;i++){
		ADCQueue[i]=xQueueCreate( 20, sizeof(uint16_t)); //creo el vector de colas para los canales ADC
		if (ADCQueue[i]==NULL)
			j=0;
	}

	for (i=0;i<DACNUM;i++){
		DACQueue[i]=xQueueCreate( MAXADCSAMPLES, sizeof(uint16_t)); //creo el vector de colas para los canales DAC
		if (DACQueue[i]==NULL)
			j=0;
	}
	DriversTestsQueue = xQueueCreate(DRIVER_TEST_QUEUE_SIZE , sizeof(FSMReg_t)); //cola que marca el estado de la maquina de estados de prueba de driver.

	if((j==0)||(UARTOutQueue==NULL)||(UARTInQueue==NULL)||(DriversTestsQueue==NULL))//chequeo que se hallan creado bien las colas
	{
		printf ("Error al crear colas");
	}

	/*----Creacion de Semaforos----*/

	j=1;
	OutQueueSem = xSemaphoreCreateBinary();		//Semaforo de cola de salida para la UART
	for (i=0;i<DACNUM;i++){
		DACWriteSem[i]=xSemaphoreCreateBinary();	//creo los semaforos que van a indicar cuando escribir
		if(DACWriteSem[i]==NULL)					//una muestra en los DAC
			j=0;
	}
	for (i=0;i<ADCNUM;i++){
		ADCReadSemCount[i]=xSemaphoreCreateCounting (MAXADCSAMPLES, 0); //semaforo contador para pedir muestras de ADC
		if(ADCReadSemCount[i]==NULL)
			j=0;
	}
	I2CMutx = xSemaphoreCreateMutex();	//semaforo para proteger el bus I2C
	xSemaphoreGive(I2CMutx);	//inicializo el bus I2C disponible
	xSemaphoreGive(OutQueueSem);	//dejo disponible la cola de salila para la UART

	if((j==0)||(OutQueueSem==NULL)||(I2CMutx))	//chequeo que se hallan creado bien los semaforos
		{
			printf ("Error al crear semaforos");
		}

}


bool_t sendStringToQueue(QueueHandle_t *Queue, QueueHandle_t *Sem,char *msg)
{
	uint8_t i=0;
	xSemaphoreTake(*Sem,portMAX_DELAY); 	//tomo el semaforo de la cola de
												//salida de la UART para que nadie mas escriba
	while (msg[i] !=  0)
	{
		xQueueSend(*Queue,&msg[i],portMAX_DELAY);	//pongo cada caracter en la cola de UART
		i++;
	}
	xSemaphoreGive(*Sem);	//Devuelvo el semaforo de la cola
}



