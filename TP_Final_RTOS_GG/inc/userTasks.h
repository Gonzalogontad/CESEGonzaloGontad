/*=============================================================================
 * Copyright (c) 2019, Eric Pernia <ericpernia@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2019/08/04
 * Version: 1
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __USER_TASKS_H__
#define __USER_TASKS_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "sapi.h"
#include "PruebaDrivers.h"
#include "queuesAndSem.h"
#include "ADS1115.h"
#include "MCP4725.h"

//#include "ADS1115.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/
#define CR 0x0d
#define LF 0x0a
#define DRIVER_TESTS_NUM 1 //numero de pruebas de drivers
#define DAC_SAMPLE_PERIOD 2	//tiempo de la ventana para escribir DAC
#define ADC_SAMPLE_PERIOD 4 //tiempo de la ventana para tomar muestras del ADC
/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

/*=====[Prototypes (declarations) of public functions]=======================*/

void salidaUart( void* taskParmPtr );  // Task declaration
void interprete( void* taskParmPtr );
void prueba( void* taskParmPtr );
void tareaDAC( void* taskParmPtr );
void tareaADC(void* taskParmPtr);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __USER_TASKS_H__ */
