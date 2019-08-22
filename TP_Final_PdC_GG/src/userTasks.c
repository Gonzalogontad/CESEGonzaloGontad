/*=============================================================================
 * Copyright (c) 2019, Eric Pernia <ericpernia@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2019/08/04
 * Version: 1
 *===========================================================================*/

/*=====[Inclusion of own header]=============================================*/
#include "sapi.h"
#include "userTasks.h"
#include "FreeRTOS.h"
#include "PruebaDrivers.h"
#include "string.h"

/*=====[Inclusions of private function dependencies]=========================*/

/*=====[Definition macros of private constants]==============================*/

/*=====[Private function-like macros]========================================*/

/*=====[Definitions of private data types]===================================*/

/*=====[Definitions of external public global variables]=====================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Prototypes (declarations) of private functions]======================*/

/*=====[Implementations of public functions]=================================*/

// Task implementation
//Recibe mensajes caracteres por una cola y los envia por la UART
void salidaUart(void* taskParmPtr)
{
	char dato;
	for (;;)
	{
		xQueueReceive(UARTOutQueue, &dato, portMAX_DELAY); //espero a que venga un dato por la cola

		uartWriteByte(UART_USB, (uint8_t) dato);	//lo escribo en la UART

	}
}

//Interprete de Comandos por UART
void interprete(void* taskParmPtr)
{
	int hash = 0;
	char dataRead;
	FSMReg_t Prueba; //Registro de FSM para enviar a la cola de la FSM de prueba de drivers
	Prueba.pruebaNum = 0;

	for (;;)
	{
		xQueueReceive(UARTInQueue, &dataRead, portMAX_DELAY);
		while (dataRead != LF) 			//Leo uno por uno los datos en el buffer
		{								//y los sumo para hacer un hash simple
			if (dataRead != CR)
				hash += (int) dataRead;	//ya que solo hay dos comandos iniciar y abortar
			xQueueReceive(UARTInQueue, &dataRead, portMAX_DELAY);
		}
		switch (hash)
		{
		case ('i' + 'n' + 'i' + 'c' + 'i' + 'a' + 'r'): //si llega un comando iniciar, inicio la prueba
		{
			Prueba.state = STATE_INIT;	//cambio el estado a INIT
			xQueueSend(DriversTestsQueue, &Prueba, portMAX_DELAY); //y lo envio a la cola de prueba de drivers
			break;
		}
		case ('a' + 'b' + 'o' + 'r' + 't' + 'a' + 'r'):	//si llega un comando abortar se detiene
		{
			//y va a condicion segura.
			Prueba.state = STATE_ABORT_ALL, xQueueReset(DriversTestsQueue);
			xQueueSendToFront(DriversTestsQueue, &Prueba, portMAX_DELAY); //envio el comando ABORT_ALL al principio
			break;														//de la cola para parar las pruebas
		}
		case 0:
			break;	//no llego ningun comando
		default:
		{
			//no se reconocio el comando
			break;
		}
		}
		hash = 0;
	}
}

//Prueba de drivers
void prueba(void* taskParmPtr)
{
	FSMReg_t ChangeFSMReg; //cambios de estado recibidos en cola
	FSMReg_t ActualFSMReg[DRIVER_TESTS_NUM]; //Vector de estados actuales de las distintas instanciaciones de la FSM
	uint8_t i;
	uint8_t j;
	char msg[30];
	for (i = 0; i < DRIVER_TESTS_NUM; i++)	//inicializo cada instanciacion de la maquina de estados
	{
		ActualFSMReg[i].pruebaNum = i;
		ActualFSMReg[i].state = STATE_IDLE;
		ActualFSMReg[i].ADCCHA = 2 * i;
		ActualFSMReg[i].ADCCHB = (2 * i) + 1;
		ActualFSMReg[i].DACCHA = i;
		ActualFSMReg[i].GPIOA = GPIO1;
		ActualFSMReg[i].i = 0;
	}
	for (;;)
	{
		if ((uxQueueSpacesAvailable(DriversTestsQueue))== DRIVER_TEST_QUEUE_SIZE) //si no hay nada para hacer en la cola
		{
			strcpy(msg, "\n\nEsperando Comando\n\n"); //envio mensaje de esperando comando
			sendStringToQueue(&UARTOutQueue, &OutQueueSem, &msg[0]);

		}
		xQueueReceive(DriversTestsQueue, &ChangeFSMReg, portMAX_DELAY);
		if (ChangeFSMReg.state == STATE_ABORT_ALL) //si llega un mensaje por la cola con el estado STATE_ABORT_ALL
		{	//significa que la cola ya fue reseteada y el unico mensaje es este
			for (i = 0; i < DRIVER_TESTS_NUM; i++)//entonces paso todas las instanciaciones de las FSM de prueba de drivers
			{		//al estado abort y lo pongo en la cola para que se ejecuten
				ActualFSMReg[i].state = STATE_ABORT;
				xQueueSendToFront(DriversTestsQueue, &ActualFSMReg[i],
						portMAX_DELAY);

			}
		}
		else //si el mensaje no es STATE_ABORT_ALL
		{
			ActualFSMReg[ChangeFSMReg.pruebaNum].state = ChangeFSMReg.state; //tomo el mensaje de cambio de estado de la cola
																			 //y actualizo el de la instanciacion correspondiente
																			 //indicada por ChangeFSMReg.pruebaNum
			FSMPruebaDrivers(&DriversTestsQueue,
					&ActualFSMReg[ChangeFSMReg.pruebaNum]);	//actualizo la maquina de estados
		}

	}
}

//esta tarea espera durante un tiempo "DAC_SAMPLE_PERIODE" para cada canal del DAC y si aparece un semaforo escribe
//el dato que le venga por cola

void tareaDAC(void* taskParmPtr)
{
	TickType_t Time; //Variable donde guardo el momento donde empieza cada ciclo de espera.
	mcpData_t DACData;
	DACData.i2cNumber = LPC_I2C0;	//numero de puerto I2C
	uint8_t i;

	for (;;)
	{
		for (i = 0; i < DACNUM; i++)
		{
			Time = xTaskGetTickCount();
			if ((xQueueReceive(DACQueue[i], &DACData.txData, (TickType_t) 0))
					== pdTRUE)
			{	//si hay un dato en la cola del canal "i" del DAC
				switch (i)
				{	//configuracion y direccion de cada canal de DAC
				case 0:
				{
					DACData.i2cSlaveAddress = 0x60; //direccion del esclavo en el bus I2C '1100000'
					DACData.pdMode = PD_NORMAL;	//modo de funcionamiento Power Down normal
					DACData.comState = dataSet_state; //estado inicial
					break;
				}
				case 1:		//los demas canales no estan implementados en esta muestra
					break;
				case 2:
					break;
				case 3:
					break;
				case 4:
					break;
				case 5:
					break;
				case 6:
					break;
				default:
					break;
				}

				if (xSemaphoreTake( I2CMutx,(TickType_t) 0 ) == pdTRUE)//si el bus I2C esta disponible
				{
					while (mcp4725NormalSend(&DACData) == FALSE); //Envio el dato por el I2C
					xSemaphoreGive(I2CMutx); //Libero el I2C
				}
				else//Si no pude enviar el dato al I2C porque estaba ocupado
					//entonces vuelvo a enviar el dato a la cola para mandarlo luego
					xQueueSendToFront(DACQueue[i], &DACData.txData,(TickType_t) (DAC_SAMPLE_PERIOD * portTICK_PERIOD_MS));

			}//Espero a que se complete el ciclo
			vTaskDelayUntil(&Time,(TickType_t) (DAC_SAMPLE_PERIOD * portTICK_PERIOD_MS));

			//luego pregunto por el siguiente canal
		}

	}
}

//Esta tarea lee los ADC secuencialmente si el semaforo correspondiente a cada uno se activó.
//la ventana de muestreo es ADC_SAMPLE_PERIOD
void tareaADC(void* taskParmPtr)
{
	uint8_t i;
	adsData_t adsData;
	TickType_t Time; //Variable donde guardo el momento donde empieza cada ciclo de espera.
	int16_t readData; //Muestra leida
	for (;;)
	{
		for (i = 0; i < (ADCNUM); i++)
		{
			Time = xTaskGetTickCount();
			if ((xSemaphoreTake(ADCReadSemCount[i], (TickType_t ) 0)) == pdTRUE)
			{ //si se tomó el semaforo del canal "i" del ADC
				switch (i)
				{	//configuracion y direccion de cada canal de ADC
				case 0:
					adsData.i2cNumber = LPC_I2C0;		//numero de puerto I2C
					adsData.i2cSlaveAddress = 0x48; //direccion del esclavo en el bus I2C '1001000'
					//uso el canal 0 en modo continuo a 860 SPS. La Vref=±4.096 V
					adsData.mode = OS_SET|MUX_A0_GND|PGA_4|MODE_CONT|DR_860|COMP_MODE_TRAD|COMP_POL_HI|COMP_LAT_DIS|COMP_QUE_DIS;
					adsData.comState = dataSet_state; //Estado de la comunicación
					break;
				case 1:
					adsData.i2cNumber = LPC_I2C0;		//numero de puerto I2C
					adsData.i2cSlaveAddress = 0x48; //direccion del esclavo en el bus I2C '1001000'
					//Modo de funcionamiento
					//uso el canal 1 en modo continuo a 860 SPS. La Vref=±4.096 V
					adsData.mode = OS_SET|MUX_A1_GND|PGA_4|MODE_CONT|DR_860|COMP_MODE_TRAD|COMP_POL_HI|COMP_LAT_DIS|COMP_QUE_DIS;
					adsData.comState = dataSet_state; //Estado de la comunicación
					break;
				case 2:		//los demas canales no estan implementados en este TP
					break;
				case 3:
					break;
				case 4:
					break;
				case 5:
					break;
				case 6:
					break;
				case 7:
					break;
				case 8:
					break;
				case 9:
					break;
				case 10:
					break;
				case 11:
					break;
				default:
					break;
				}
				//intento tomar el bus I2C
				if (xSemaphoreTake(I2CMutx,(TickType_t) 1 * portTICK_PERIOD_MS)== pdTRUE) //puedo esperar hasta 1 ms
				{
					//si el bus I2C esta disponible
					while (ads1115Init(&adsData) == FALSE); //Realiza la secuencia de envio por el bus I2C
					xSemaphoreGive(I2CMutx); //Libero el I2C
					vTaskDelayUntil(&Time,(TickType_t) (3 * portTICK_PERIOD_MS)); //espero 3 miliseg hasta que termine la conversion
					if (xSemaphoreTake(I2CMutx,(TickType_t) 1 * portTICK_PERIOD_MS) == pdTRUE) //puedo esperar hasta 1 ms
					{
						//si el bus I2C esta disponible
						while (ads1115ReadRegister(&adsData,ADS1115_CONV_REG) == FALSE); //pido el valor de la muestra al ADC
						xSemaphoreGive(I2CMutx); //Libero el I2C
						readData = (int16_t) (((adsData.rxData[0]) << 8)+ (adsData.rxData[1])); //armo el dato
						xQueueSend(ADCQueue[i], &readData,1 * portTICK_PERIOD_MS); // y lo envio a la cola del ADC
					}
					else
						xSemaphoreGive(ADCReadSemCount[i]); //Si no pude enviar el dato al I2C porque estaba ocupado
															//entonces vuelvo a enviar el semaforo contador una solicitud de muestra
				}
				else
					xSemaphoreGive(ADCReadSemCount[i]); //Si no pude enviar el dato al I2C porque estaba ocupado
														//entonces vuelvo a enviar el semaforo contador una solicitud de muestra

			}
			//else
			vTaskDelayUntil(&Time,(TickType_t) (ADC_SAMPLE_PERIOD * portTICK_PERIOD_MS)); //Espero a que se complete el ciclo
			//luego pregunto por el siguiente canal
		}

	}
}

/*=====[Implementations of interrupt functions]==============================*/

/*=====[Implementations of private functions]================================*/

