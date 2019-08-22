#include "PruebaDrivers.h"
#include "string.h"
#include "sapi_convert.h"
#include "sapi.h"

char msg[80]; //String para enviar mensajes por la cola de UART
char msg2[10];//String auxiliar

//Esta funcion recibe un puntero de cola y un registro de la maquina de estados que debe actualizar.
//En la cola envia el siguiente estado a ejecutar.
//Esto se hace asi para que se puedan enviar cambios desde distintas tareas que compartan la cola.

void FSMPruebaDrivers(QueueHandle_t* Queue, FSMReg_t *FSMReg)
{
	uint16_t i = 0; 		//Variable auxiliar para lazos

	int16_t ADCDataTemp;	//Variables auxiliar par los valores del ADC
	uint16_t DACValue;		//Valor a escribir en el DAC
	volatile uint32_t tempTick;		//Guardo los valores del Tick counter para medir tiempos
	char uartBuff[10];

	/*------Maquina de estados------*/

	switch (FSMReg->state)
	{
	case STATE_INIT:
	{	//inicialización
		gpioWrite(FSMReg->GPIOA, FALSE); //Le quito la alimentación al driver (no deberia estar activa en
										 //este punto pero es buena practica asegurarse.
		gpioWrite(LED1, FALSE);	//El led1 indica si el GPIOA esta activo (solo para la demostracion)

		//Armo el mensaje de inicio de prueba y lo envio a la cola de salida de la UART
		//No usar sprintf porque llena la memoria
		strcpy(msg, "Inicio prueba N ");
		msg2[0] = (char) (FSMReg->pruebaNum + '1');
		msg2[1] = CR;
		msg2[2] = LF;
		msg2[3] = 0;
		strcat(msg, msg2);
		sendStringToQueue(&UARTOutQueue, &OutQueueSem, &msg[0]);
		FSMReg->state = STATE_DAC_SET;	//paso al siguiente estado
		xQueueSend(*Queue, FSMReg, portMAX_DELAY); //lo envio a la cola para que se ejecute luego
		for (i = 0; i < 3; i++)
		{
			FSMReg->ADCCHAPROM[i] = 0;//inicializo los vectores de medicion (ADC)
			FSMReg->ADCCHBPROM[i] = 0;//Aqui se van a guardar los valores promedio de las muestras tomadas
		}
		FSMReg->i = 0; //indice auxiliar de la instanciacion actual de la maquina de estados
		break;
	}
	case STATE_DAC_SET:
	{
		switch (FSMReg->i)
		{ //el indice FSMReg->i marca cual de las tres mediciones se esta ejecutando 0, 10 y 20
		case 0:
			DACValue = (uint16_t) (DAC_SCALE * DACVAL0); 	//Valores a setear en el DAC convertidos a uint16
			break;											//para cada medicion
		case 10:
			DACValue = (uint16_t) (DAC_SCALE * DACVAL1);
			break;
		case 20:
			DACValue = (uint16_t) (DAC_SCALE * DACVAL2);
			break;
		}

		xQueueSend(DACQueue[FSMReg->pruebaNum], &DACValue, portMAX_DELAY); //pongo en la cola del DAC el dato a escribir

		FSMReg->tickRegister = xTaskGetTickCount(); //tomo el estado del tick para la demora
		FSMReg->state = STATE_WAIT_100MS;	//paso al siguiente estado
		xQueueSend(*Queue, FSMReg, portMAX_DELAY); //lo envio a la cola para que se ejecute
		break;
	}
	case STATE_WAIT_100MS: 	//hago un retardo de 100mS, no uso el vTaskDelay porque quiero que se ejecuten
	{						//otras etapas de la misma tarea mientras se hace la espera.
		tempTick = xTaskGetTickCount(); //Tomo el estado actual del tick y luego comparo con el valor al inicio
		if (tempTick >= FSMReg->tickRegister) //chequeo que el Tick counter no se haya reiniciado
		{
			if ((tempTick - FSMReg->tickRegister)
					> (100 * ((uint32_t) portTICK_PERIOD_MS))) //si pasaron 100ms
			{
				//condicion de salida
				FSMReg->tickRegister = xTaskGetTickCount(); //guardo el tick por si se necesita reiniciar
				FSMReg->state = STATE_DRIVER_EN_AND_DELAY;	//paso al siguiente estado
				xQueueSend(*Queue, FSMReg, portMAX_DELAY); 	//lo envio a la cola para que se ejecute
			}
			else
				xQueueSend(*Queue, FSMReg, portMAX_DELAY); //no hay cambio de estado, envio lo mismo a la cola

		}
		else //el Tick counter se reinicio
		{
			if ((tempTick + (0xFFFF - FSMReg->tickRegister))
					> (100 * ((uint32_t) portTICK_PERIOD_MS))) //si pasaron 100ms
			{
				//condicion de salida
				FSMReg->tickRegister = xTaskGetTickCount(); //guardo el tick por si se necesita reiniciar
				FSMReg->state = STATE_DRIVER_EN_AND_DELAY;//paso al siguiente estado
				xQueueSend(*Queue, FSMReg, portMAX_DELAY); //lo envio a la cola para que se ejecute

			}
			else
				xQueueSend(*Queue, FSMReg, portMAX_DELAY); //no hay cambio de estado, envio lo mismo a la cola

		}

		break;
	}
	case STATE_DRIVER_EN_AND_DELAY:
	{
		switch (FSMReg->i)
		{ //el indice ademas de indicar la medicion a realizar tambien marca las veces que se hizo el delay
		case 9:	//En los 3 estados siguientes debo empezar con la toma de muestras de ADCs
		case 19://porque ya pasaron los 1000ms
		case 29:
			FSMReg->state = STATE_REQUEST_ADCS;	//paso al siguiente estado
			break;
		case 0:
			gpioWrite(FSMReg->GPIOA, TRUE);	 //Enciendo el driver
			gpioWrite(LED1, TRUE); 			//enciendo el led para que se vea que el driver esta encendido
		case 1:	//en los siguentes casos repito la demora de 100ms
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
			FSMReg->state = STATE_WAIT_100MS;	//vuelvo a hacer la demora
			break;
		default:
			printf("ERROR"); //no debe llegar nunca aqui
			while (1)
				;

		}
		FSMReg->i++; //incremento el indice para el siguiente reingreso a la FSM
		xQueueSend(*Queue, FSMReg, portMAX_DELAY); //Envio el estado la cola para que se ejecute luego
		break;

	}
	case STATE_REQUEST_ADCS:	//envio al semaforo contador de ADCs un pedido de 10 muestras para promediar
	{
		for (i = 0; i < 10; i++)
		{
			xSemaphoreGive(ADCReadSemCount[FSMReg->ADCCHA]); //pido 10 muestras a dos canales de ADC
			xSemaphoreGive(ADCReadSemCount[FSMReg->ADCCHB]);
		}
		//condicion de salida
		FSMReg->state = STATE_READ_ADCS;	//paso al siguiente estado
		xQueueSend(*Queue, FSMReg, portMAX_DELAY); //lo envio a la cola para que se ejecute luego
		break;

	}
	case STATE_READ_ADCS:	//Esperar a que las muestras esten en la cola del ADC y promediarlas
	{
		switch (FSMReg->i)
		{
		case 10:
		case 20:
			FSMReg->state = STATE_DAC_SET;//si todavia no llegue a la tercer medicion vuelvo al
										  //principio para hacer la siguiente medicion
		case 30:
			if (FSMReg->i == 30)				//si estoy en la 3er medicion
				FSMReg->state = STATE_CHECK;	//paso al siguiente estado

			for (i = 0; i < 10; i++)	//calculo los promedios de las mediciones
			{
				xQueueReceive(ADCQueue[FSMReg->ADCCHA], &ADCDataTemp,portMAX_DELAY);
				FSMReg->ADCCHAPROM[(FSMReg->i / 10) - 1] += (ADCDataTemp); //el indice FSMReg->i hay que dividirlo por 10 porque va de 10 en 10.
				xQueueReceive(ADCQueue[FSMReg->ADCCHB], &ADCDataTemp,portMAX_DELAY);
				FSMReg->ADCCHBPROM[(FSMReg->i / 10) - 1] += (ADCDataTemp);
			}
			FSMReg->ADCCHAPROM[(FSMReg->i / 10) - 1] =FSMReg->ADCCHAPROM[(FSMReg->i / 10) - 1] / 10;
			FSMReg->ADCCHBPROM[(FSMReg->i / 10) - 1] =FSMReg->ADCCHBPROM[(FSMReg->i / 10) - 1] / 10;
			xQueueSend(*Queue, FSMReg, portMAX_DELAY); //Envio el estado a la cola para que se ejecute luego
			break;
		default:
			printf("ERROR"); //no debe llegar nunca aqui
			while (1);
			break;
		}

		break;
	}

	case STATE_CHECK:	//evaluar los resultados y enviar por la UART
		strcpy(msg, "Resultados Prueba N ");
		int64ToString((FSMReg->pruebaNum + 1), msg2, 10);
		strcat(msg, msg2);

		if ((ADC_A0MIN < FSMReg->ADCCHAPROM[0])	//si la medicion 1 del canal A esta entre el valor Minimo
				&& (FSMReg->ADCCHAPROM[0] < ADC_A0MAX )//y el valor Maximo
				&& (ADC_B0MIN < FSMReg->ADCCHBPROM[0])	// y lo mismo para el canal B
				&& (FSMReg->ADCCHBPROM[0] < ADC_B0MAX ))
			strcat(msg, "\n\tMedicion 1 OK \n");		//entonces la medicion 1 esta OK
		else
			strcat(msg, "\n\tMedicion 1 NO PASA \n");	//Igual para la medicion 2 y 3
		if ((ADC_A1MIN < FSMReg->ADCCHAPROM[1])
				&& (FSMReg->ADCCHAPROM[1] < ADC_A1MAX )
				&& (ADC_B1MIN < FSMReg->ADCCHBPROM[1])
				&& (FSMReg->ADCCHBPROM[1] < ADC_B1MAX ))
			strcat(msg, "\tMedicion 2 OK \n");
		else
			strcat(msg, "\tMedicion 2 NO PASA \n");
		if ((ADC_A2MIN < FSMReg->ADCCHAPROM[2])
				&& (FSMReg->ADCCHAPROM[2] < ADC_A2MAX )
				&& (ADC_B2MIN < FSMReg->ADCCHBPROM[2])
				&& (FSMReg->ADCCHBPROM[2] < ADC_B2MAX ))
			strcat(msg, "\tMedicion 3 OK \n");
		else
			strcat(msg, "\tMedicion 3 NO PASA \n");
		//los mensajes de prueba OK y de NO PASA estan concatenados en un solo String para enviarlos juntos a la UART
		sendStringToQueue(&UARTOutQueue, &OutQueueSem, &msg[0]);

		FSMReg->state = STATE_IDLE;	//paso al estado IDLE
		xQueueSend(*Queue, FSMReg, portMAX_DELAY); //lo envio a la cola para que se ejecute
		break;

	case STATE_ABORT:
		gpioWrite(FSMReg->GPIOA, FALSE); //Apago el driver
		gpioWrite(LED1, FALSE);
		strcpy(msg, "\n\nPrueba N ");
		int64ToString(FSMReg->pruebaNum, msg2, 10);
		strcat(msg, msg2);
		strcat(msg, " detenida\n\n");
		sendStringToQueue(&UARTOutQueue, &OutQueueSem, &msg[0]); //envio mensaje de Prueba abortada

		FSMReg->state = STATE_IDLE;	//paso al estado IDLE
		xQueueSend(*Queue, FSMReg, portMAX_DELAY); //lo envio a la cola para que se ejecute
		break;

	case STATE_IDLE: //condicion segura
		DACValue = 0; //pongo el DAC en cero
		xQueueSend(DACQueue[FSMReg->pruebaNum], &DACValue, portMAX_DELAY); //pongo en la cola el dato a escribir
		gpioWrite(FSMReg->GPIOA, FALSE); //Apago el driver
		gpioWrite(LED1, FALSE);
		break;

	default: //no deberia llegar nunca aca
		gpioWrite(FSMReg->GPIOA, FALSE); //Apago el driver
		gpioWrite(LED1, FALSE);
		printf("ERROR");
		break;
	}
	return;
}
