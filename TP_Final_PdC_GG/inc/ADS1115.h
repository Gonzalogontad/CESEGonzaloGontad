/*
 * ADS1115.h
 *
 *  Created on: 27 jul. 2019
 *      Author: gonzalo
 */

#ifndef ADS1115_H_
#define ADS1115_H_

/*El ADS1115 adopta una direccion en el bus I2C segun a que pin se conecta el pin ADDR
 *PIN Address
 *GND 1001000
 *VDD 1001001
 *SDA 1001010
 *SCL 1001011
 */
#define ADS1115_ADDR_PIN_TO_VDD    0x49 //Pin address conectado al pin de Alimentación
#define ADS1115_ADDR_PIN_TO_GND    0x48 //Pin address conectado al pin de Masa
#define ADS1115_ADDR_PIN_TO_SCL    0x4B //Pin address conectado al pin SCL
#define ADS1115_ADDR_PIN_TO_SDA    0x4A //Pin address conectado al pin SDA

///----Registros del ADS1115----

#define ADS1115_CONV_REG       	0x00 //Convertion register, es donde se guarda el valor de la conversion
#define ADS1115_CONFIG_REG     	0x01 //Config Register,
#define ADS1115_LOWTHRESH_REG  	0x02 //Umbral inferior para la comparacion
#define ADS1115_HIGHTHRESH_REG  0x03 //Umbral superior para la comparacion

//----Mascaras para Config Register----

/*Config Register
Bit		Field		Type	Reset	Description
15 		OS 			R/W 	1h		Operational status or single-shot conversion start
									This bit determines the operational status of the device. OS can only be written
									when in power-down state and has no effect when a conversion is ongoing.
									When writing:
									0 : No effect
									1 : Start a single conversion (when in power-down state)
									When reading:
									0 : Device is currently performing a conversion
									1 : Device is not currently performing a conversion*/
#define OS_SET		0x8000
#define OS_CLEAR	0x7FFF //usar AND
#define OS_RESET
/*
14:12 	MUX[2:0]	 R/W	 0h		Input multiplexer configuration (ADS1115 only)
									These bits configure the input multiplexer. These bits serve no function on the
									ADS1113 and ADS1114.
									000 : AINP = AIN0 and AINN = AIN1 (default)
									001 : AINP = AIN0 and AINN = AIN3
									010 : AINP = AIN1 and AINN = AIN3
									011 : AINP = AIN2 and AINN = AIN3
									100 : AINP = AIN0 and AINN = GND
									101 : AINP = AIN1 and AINN = GND
									110 : AINP = AIN2 and AINN = GND
									111 : AINP = AIN3 and AINN = GND*/
#define MUX_A0_A1	0<<12
#define MUX_A0_A3	1<<12
#define MUX_A1_A3	2<<12
#define MUX_A2_A3	3<<12
#define MUX_A0_GND	4<<12
#define MUX_A1_GND	5<<12
#define MUX_A2_GND	6<<12
#define MUX_A3_GND	7<<12
/*
11:9 	PGA[2:0] 	R/W 	2h		Programmable gain amplifier configuration
									These bits set the FSR of the programmable gain amplifier. These bits serve no
									function on the ADS1113.
									000 : FSR = ±6.144 V
									(1)
									001 : FSR = ±4.096 V
									(1)
									010 : FSR = ±2.048 V (default)
									011 : FSR = ±1.024 V
									100 : FSR = ±0.512 V
									101 : FSR = ±0.256 V
									110 : FSR = ±0.256 V
									111 : FSR = ±0.256 V*/
#define PGA_6		0<<9
#define PGA_4		1<<9
#define PGA_2		2<<9
#define PGA_1		3<<9
#define PGA_05		4<<9
#define PGA_025	5<<9
/*
8 		MODE 		R/W 	1h		Device operating mode
									This bit controls the operating mode.
									0 : Continuous-conversion mode
									1 : Single-shot mode or power-down state (default)*/
#define MODE_CONT		0<<8
#define MODE_SINGLE			1<<8
/*
7:5 	DR[2:0] 	R/W 	4h		Data rate
									These bits control the data rate setting.
									000 : 8 SPS
									001 : 16 SPS
									010 : 32 SPS
									011 : 64 SPS
									100 : 128 SPS (default)
									101 : 250 SPS
									110 : 475 SPS
									111 : 860 SPS*/
#define DR_8		0<<5
#define DR_16		1<<5
#define DR_32		2<<5
#define DR_64		3<<5
#define DR_128		4<<5
#define DR_250		5<<5
#define DR_475		6<<5
#define DR_860		7<<5

/*
4 		COMP_MODE 	R/W 	0h		Comparator mode (ADS1114 and ADS1115 only)
									This bit configures the comparator operating mode. This bit serves no function on
									the ADS1113.
									0 : Traditional comparator (default)
									1 : Window comparator*/
#define COMP_MODE_TRAD		0<<4
#define COMP_MODE_WINDOW		1<<4
/*
3 		COMP_POL 	R/W 	0h		Comparator polarity (ADS1114 and ADS1115 only)
									This bit controls the polarity of the ALERT/RDY pin. This bit serves no function on
									the ADS1113.
									0 : Active low (default)
									1 : Active high*/
#define COMP_POL_HI		0<<3
#define COMP_POL_LO		1<<3

/*
2 		COMP_LAT 	R/W 	0h		Latching comparator (ADS1114 and ADS1115 only)
									This bit controls whether the ALERT/RDY pin latches after being asserted or
									clears after conversions are within the margin of the upper and lower threshold
									values. This bit serves no function on the ADS1113.
									0 : Nonlatching comparator . The ALERT/RDY pin does not latch when asserted
									(default).
									1 : Latching comparator. The asserted ALERT/RDY pin remains latched until
									conversion data are read by the master or an appropriate SMBus alert response
									is sent by the master. The device responds with its address, and it is the lowest
									address currently asserting the ALERT/RDY bus line.*/
#define COMP_LAT_DIS	0<<2
#define COMP_LAT_EN		1<<2

/*1:0 	COMP_QUE[1:0] R/W 	3h		Comparator queue and disable (ADS1114 and ADS1115 only)
									These bits perform two functions. When set to 11, the comparator is disabled and
									the ALERT/RDY pin is set to a high-impedance state. When set to any other
									value, the ALERT/RDY pin and the comparator function are enabled, and the set
									value determines the number of successive conversions exceeding the upper or
									lower threshold required before asserting the ALERT/RDY pin. These bits serve
									no function on the ADS1113.
									00 : Assert after one conversion
									01 : Assert after two conversions
									10 : Assert after four conversions
									11 : Disable comparator and set ALERT/RDY pin to high-impedance (default)*/
#define COMP_QUE_1		0
#define COMP_QUE_2		1
#define COMP_QUE_4		2
#define COMP_QUE_DIS	3

//----Estados de la comunicación----
enum {
	dataSet_state,
	sending_state,
	ready_state,
};

//----Estructura de cada ADS1115 conectado----
typedef struct {
	LPC_I2C_T 	*i2cNumber;			//numero de puerto I2C
	uint8_t  	i2cSlaveAddress; 	//direccion del esclavo en el bus I2C
	uint16_t	mode;				//modo de funcionamiento (registro config)
	uint8_t 	comState;	//Estado de la comunicación
	uint8_t 	rxData [2]; //datos recibidos

}adsData_t;

//----Funciones de comunicacion con el ADS1115----

//Esta función escribe el registro Config  según la información que le paso
//por un puntero a la estructura que contiene todos los datos de un ADS1115
//Si devuelve TRUE es que terminó de enviar
//si devuelve FALSE hay que volver a llamar a la función hasta que devuelva TRUE porque no termino

bool_t ads1115Init ( adsData_t *);

//Esta función lee un registro que le indico mediante regNumber en el ADS que le indico con la estructura adsDataX
//Si devuelve TRUE es que terminó de leer
//si devuelve FALSE hay que volver a llamar a la función hasta que devuelva TRUE porque no termino
bool_t ads1115ReadRegister ( adsData_t *adsDataX,uint8_t regNumber );



#endif
