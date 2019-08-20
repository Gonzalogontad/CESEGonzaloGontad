/*=============================================================================
 * Copyright (c) 2019, Eric Pernia <ericpernia@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2019/08/04
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "TP_Final_RTOS_GG.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "UARTInterrupt.h"
#include "semphr.h"
#include "queuesAndSem.h"
#include "userTasks.h"


/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
   boardInit();
   gpioInit( GPIO1, GPIO_OUTPUT );
   /* Inicializar la UART_USB junto con las interrupciones de Rx */
   uartConfig(UART_USB, 115200);
   // Seteo un callback al evento de recepcion y habilito su interrupcion
   uartCallbackSet(UART_USB, UART_RECEIVE, uartUsbReceiveCallback, NULL);
   // Habilito todas las interrupciones de UART_USB
   uartInterrupt(UART_USB, TRUE);

   i2cInit(I2C0, 800000); //Configuro el I2C para funcionar a 800khz
   // Create a task in freeRTOS with dynamic memory
   xTaskCreate(
      salidaUart,                     // Function that implements the task.
      (const char *)"Salida UART",     // Text name for the task.
      configMINIMAL_STACK_SIZE*2, // Stack size in words, not bytes.
      0,                          // Parameter passed into the task.
      tskIDLE_PRIORITY+1,         // Priority at which the task is created.
      0                           // Pointer to the task created in the system
   );
   xTaskCreate(
	  interprete,                     // Function that implements the task.
      (const char *)"Interprete",     // Text name for the task.
      configMINIMAL_STACK_SIZE*2, // Stack size in words, not bytes.
      0,                          // Parameter passed into the task.
      tskIDLE_PRIORITY+1,         // Priority at which the task is created.
      0                           // Pointer to the task created in the system
   );
   xTaskCreate(
	  prueba,                     // Function that implements the task.
      (const char *)"Prueba",     // Text name for the task.
      configMINIMAL_STACK_SIZE*2, // Stack size in words, not bytes.
      0,                          // Parameter passed into the task.
      tskIDLE_PRIORITY+1,         // Priority at which the task is created.
      0                           // Pointer to the task created in the system
   );
   xTaskCreate(
	  tareaDAC,                     // Function that implements the task.
      (const char *)"DAC",     // Text name for the task.
      configMINIMAL_STACK_SIZE*2, // Stack size in words, not bytes.
      0,                          // Parameter passed into the task.
      tskIDLE_PRIORITY+2,         // Priority at which the task is created.
      0                           // Pointer to the task created in the system
   );
   xTaskCreate(
	  tareaADC,                     // Function that implements the task.
      (const char *)"ADC",     // Text name for the task.
      configMINIMAL_STACK_SIZE*2, // Stack size in words, not bytes.
      0,                          // Parameter passed into the task.
      tskIDLE_PRIORITY+2,         // Priority at which the task is created.
      0                           // Pointer to the task created in the system
   );

   //Creo colas y semaforos
   queuesAndSemInit();

   vTaskStartScheduler(); // Initialize scheduler

   while( TRUE ); // If reach heare it means that the scheduler could not start

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 
   // case of a PC program.
   return 0;
}
