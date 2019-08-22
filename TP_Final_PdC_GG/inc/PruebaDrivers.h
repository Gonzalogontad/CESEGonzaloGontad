/*
 * PruebaDrivers.h
 *
 *  Created on: 10 ago. 2019
 *      Author: gonzalo
 */

#ifndef PRUEBADRIVERS_H_
#define PRUEBADRIVERS_H_

#include "queuesAndSem.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "sapi.h"

#define CR 0x0d
#define LF 0x0a
#define DAC_SCALE (float)4096/3.3 //Multiplicar por el valor en Volts para obtener el N° para el DAC
#define ADC_SCALE 8000 // Tomar la muestra y dividir por ADC_SCALE para obtener el valor en Volts (1/125uV)
#define DAC_SCALE (float)4096/3.3 //Multiplicar por el valor en Volts para obtener el N° para el DAC
#define ADC_SCALE 8000 // Tomar la muestra y dividir por ADC_SCALE para obtener el valor en Volts (1/125uV)
#define DACVAL0 1	//seteo el valor del DAC en volts de cada prueba
#define DACVAL1 2
#define DACVAL2 3
#define ADC_A0MAX	(int)(1.2*ADC_SCALE)	//Valores maximos y minimos para los ADC
#define ADC_A0MIN	(int)(0.8*ADC_SCALE)
#define ADC_A1MAX	(int)(2.2*ADC_SCALE)
#define ADC_A1MIN	(int)(1.8*ADC_SCALE)
#define ADC_A2MAX	(int)(3.2*ADC_SCALE)
#define ADC_A2MIN	(int)(2.8*ADC_SCALE)
#define ADC_B0MAX	(int)(0.6*ADC_SCALE)
#define ADC_B0MIN	(int)(0.4*ADC_SCALE)
#define ADC_B1MAX	(int)(1.1*ADC_SCALE)
#define ADC_B1MIN	(int)(0.9*ADC_SCALE)
#define ADC_B2MAX	(int)(1.6*ADC_SCALE)
#define ADC_B2MIN	(int)(1.4*ADC_SCALE)
//Tipo de datos de estados de las FSM
typedef enum{
   STATE_INIT,
   STATE_DAC_SET,
   STATE_WAIT_100MS,
   STATE_DRIVER_EN_AND_DELAY,
   STATE_WAIT_2S,
   STATE_REQUEST_ADCS,
   STATE_READ_ADCS,
   STATE_CHECK,
   STATE_ABORT,
   STATE_ABORT_ALL, //aborta todas las pruebas (solo puede llegar esta orden desde el interprete de comandos)
   STATE_IDLE,	//condicion segura
} fsmState_t;

//Tipo de dato de variables de estado y recursos de la FSM de prueba de drivers
typedef struct {
	uint8_t 	pruebaNum;		//Numero de prueba en ejecucion
	fsmState_t 	state;			//estado actual de la FSM
	uint32_t 	tickRegister;	//Registro de ticks para poder medir tiempos
	uint8_t		ADCCHA;			//Canales del ADC asignados a esta instanciacion de la FSM
	uint8_t		ADCCHB;
	int32_t		ADCCHAPROM [3]; //promedio de las muestras del ADC CHA para las tres mediciones
	int32_t		ADCCHBPROM [3]; //promedio de las muestras del ADC CHB para las tres mediciones
	uint8_t		DACCHA;		//Canal del DAC asignado a esta instanciacion de la FSM
	gpioMap_t	GPIOA;		//GPIOs asignados a esta instanciacion de la FSM
	uint8_t		i;			//El indice i marca cual de las tres mediciones se esta ejecutando 0, 10 y 20 y
							//ademas indica cuantas veces se ejecuto la demora de 100ms
} FSMReg_t;

void FSMPruebaDrivers(QueueHandle_t* ,FSMReg_t *); //maquina de estados de prueba de drivers


#endif /* PRUEBA_DRIVERS_H_ */
