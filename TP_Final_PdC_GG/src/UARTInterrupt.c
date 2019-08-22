/*
 * UARTInterrupt.c
 *
 *  Created on: 9 ago. 2019
 *      Author: gonzalo
 */
#include "UARTInterrupt.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "sapi.h"
#include "queuesAndSem.h"

//La interrupcion de la UART recibe un dato y lo pone en la cola de la UART para que el interprete de
//comandos procece la información
void uartUsbReceiveCallback( void *unused )
{
	char dataReceived;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	dataReceived = uartRxRead(UART_USB); //leo el dato
	xQueueSendFromISR (UARTInQueue,&dataReceived,&xHigherPriorityTaskWoken);//lo pongo en la cola
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //aviso al RTOS si hay cambios

}



