/*
 * ADS1115.h
 *
 *  Created on: 27 jul. 2019
 *      Author: gonzalo
 */

#ifndef ADS1115_H_
#define ADS1115_H_

enum {
	convertionReg,
	configReg,
	loTreshReg,
	hiTreshReg,
};
enum {
	dataSet_state,
	sending_state,
	ready_state,
};


typedef struct {
	LPC_I2C_T *i2cNumber;			//numero de puerto I2C
	uint8_t  i2cSlaveAddress; 	//direccion del esclavo en el bus I2C
	uint8_t modeHi;				//modo de funcionamiento parte alta
	uint8_t modeLo;				//modo de funcionamiento parte baja
	uint8_t comState;	//Estado de la comunicación
	uint8_t rxData [2]; //datos recibidos

}adsData_t;



bool_t ads1115Init ( adsData_t *);//inicializa el ADC
bool_t ads1115Read ( adsData_t *);//Lee 2 bytes del ADC




#endif /* PROYECTOS_CESE_TP1PDC9COGG_INC_ADS1115_H_ */
