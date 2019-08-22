/*
 * MCP4725.h
 *
 *  Created on: 27 jul. 2019
 *      Author: gonzalo
 */

#ifndef MCP4725_H_
#define MCP4725_H_

//----Comandos MCP4725----
#define WRITE_DAC_REG 			0x02<<5 //Escribir registro del DAC, C2 = 0, C1 = 1, C0 = 0
#define WRITE_DAC_REG_EEPROM 	0x03<<5 //Escribir registro del DAC y la EEPROM, C2 = 0, C1 = 1, C0 = 1
#define FASTE_WR_DAC_REG		0x0 <<6	//Escribir registro del DAC en modo rapido, C2 = 0, C1 = 0
#define PD_NORMAL				0x0 <<1 //Power down en modo normal (salida activa)
#define PD_1K					0x1 <<1	//Power down con salida de 1k
#define PD_100K					0x2 <<1 //Power down con salida de 100k
#define PD_500K					0x3 <<1 //Power down con salida de 500k
#define FAST_PD_NORMAL			0x0 <<4 //Power down en modo normal (salida activa) para el modo de escritura rapido
#define FAST_PD_1K				0x1 <<4	//Power down con salida de 1k para el modo de escritura rapido
#define FAST_PD_100K			0x2 <<4 //Power down con salida de 100k para el modo de escritura rapido
#define FAST_PD_500K			0x3 <<4 //Power down con salida de 500k para el modo de escritura rapido

#ifndef ADS1115_H_
enum{
	dataSet_state, sending_state
};
#endif


typedef struct
{
	LPC_I2C_T *i2cNumber;			//numero de puerto I2C
	uint8_t i2cSlaveAddress; 	//direccion del esclavo en el bus I2C
	uint8_t pdMode;				//modo de funcionamiento Power Down
	uint8_t comState;	//Estado de la comunicación
	uint16_t txData; //datos a enviar
	uint16_t rxData; //datos recibidos

} mcpData_t;


//Funcion que escribe un valor en el DAC en modo Normal (se envian 4 bytes en total)
//Si devuelve TRUE es que terminó de enviar
//si devuelve FALSE hay que volver a llamar a la función hasta que devuelva TRUE porque no termino
bool_t mcp4725NormalSend(mcpData_t *); //inicializa el ADC

//Funcion que escribe un valor en el DAC en modo Fast (se envian 3 bytes en total)
//Si devuelve TRUE es que terminó de enviar
//si devuelve FALSE hay que volver a llamar a la función hasta que devuelva TRUE porque no termino
bool_t mcp4725FastSend(mcpData_t *);


#endif
