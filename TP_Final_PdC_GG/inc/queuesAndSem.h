/*
 * queuesAndSem.h
 *
 *  Created on: 6 ago. 2019
 *      Author: gonzalo
 */
#include "FreeRTOS.h"
#include "semphr.h"
#include "PruebaDrivers.h"
#include "sapi.h"
#ifndef _QUEUESANDSEM_H_
#define _QUEUESANDSEM_H_

#define ADCNUM 2 //cantidad de canales de ADC
#define DACNUM 1 //cantidad de canales de DAC
#define MAXADCSAMPLES 20 //Cantidad maxima de muestras a pedirle a un ADC
#define DRIVER_TEST_QUEUE_SIZE (UBaseType_t)7 	//Cantidad de elementos de la cola de prueba de drivers un
									//elemeto para cada prueba (6) + un elemento para un comando desde UART (1)

QueueHandle_t UARTOutQueue; //cola de datos salientes de la interfaz UART
QueueHandle_t UARTInQueue; //cola de datos entrantes de la interfaz UART
QueueHandle_t ADCQueue[ADCNUM]; //vector de colas de ADC
QueueHandle_t DACQueue[DACNUM]; //vector de colas de DAC
QueueHandle_t DriversTestsQueue; //Cola que indica el estado de la maquina que hay que ejecutar y la dispara
SemaphoreHandle_t OutQueueSem; //semaforo para escribir un mensaje en la cola de salida de la UART
SemaphoreHandle_t DACWriteSem[DACNUM]; //Vector de semaforos que indica que hay que escribir un DAC
SemaphoreHandle_t ADCReadSemCount[ADCNUM]; //Vector de semaforos contadores que indican cuando tomar una muestra de ADC
SemaphoreHandle_t I2CMutx;
void queuesAndSemInit(void);
bool_t sendStringToQueue(QueueHandle_t *, QueueHandle_t *,char *);





#endif /* PROYECTOS_CESE_RTOS_2019_9CO_TP_FINAL_RTOS_GG_INC_QUEUESANDSEM_H_ */
