/*
 * MCP4725.h
 *
 *  Created on: 27 jul. 2019
 *      Author: gonzalo
 */

#ifndef MCP4725_H_
#define MCP4725_H_

#ifndef ADS1115_H_
enum
{
	dataSet_state, sending_state, ready_state,
};
#endif

typedef struct
{
	LPC_I2C_T *i2cNumber;			//numero de puerto I2C
	uint8_t i2cSlaveAddress; 	//direccion del esclavo en el bus I2C
	uint8_t mode;				//modo de funcionamiento
	uint8_t comState;	//Estado de la comunicación
	uint16_t txData; //datos a enviar

} mcpData_t;

bool_t mcp4725NormalSend(mcpData_t *); //inicializa el ADC


#endif /* PROYECTOS_CESE_TP1PDC9COGG_INC_ADS1115_H_ */
