#include <sapi.h>

#include <ADS1115.h>

//Esta funcion escribe el registro Config  segun la informacion que le paso
//por un puntero a la estructura que contiene todos los datos de un ADS1115
//Si devuelve TRUE es que termino de enviar
//si devuelve FALSE hay que volver a llamar a la funcion hasta que devuelva TRUE porque no termino
bool_t ads1115Init ( adsData_t *adsDataX)
	{

	I2CM_XFER_T i2cData; //Estructura que le voy a pasar al handler de transmision del I2C
	uint8_t txBuffer[3];
	static uint32_t ret=0; //valor de retorno
	uint8_t modeHI;
	uint8_t modeLO;



	switch (adsDataX->comState)
	{
		case dataSet_state:	//Enviar el primer byte, indica a que registro quiero acceder
		{
			modeHI = adsDataX->mode >>8;	//Reparto la variable mode en 2 bytes
			modeLO = ((adsDataX->mode) & 0x00FF);
			txBuffer[0] = (uint8_t)ADS1115_CONFIG_REG; 	//Indico que voy a escribir el registro de configuración
			txBuffer[1] = modeHI; 		//le paso la parte alta al handler
			txBuffer[2] = modeLO;		//le paso la parte baja	al handler
			//Copio toda la informacion a la estructura i2cData para pasarsela al handler del bus I2C
			i2cData.slaveAddr = adsDataX->i2cSlaveAddress; //
			i2cData.options   = 0;
			i2cData.status    = 0;
			i2cData.txBuff    = txBuffer;
			i2cData.txSz      = 6;
			i2cData.rxBuff    = 0;
			i2cData.rxSz      = 0;


				   Chip_I2CM_Xfer(adsDataX->i2cNumber, &i2cData); //le paso el puerto I2C que quiero escribir
																   //y la informacion con la dreccion del esclavo
				   adsDataX->comState = sending_state; //paso al siguiente estado
				   ret = FALSE;
				   break;
		}
		case sending_state:
		{	//Ejecuto el handler hasta que se termine de transmitir
			if ( Chip_I2CM_StateChanged(adsDataX->i2cNumber) == 0)
			{
				ret = FALSE;
			}

			//si hay cambio en el controlador de I2C
			ret = Chip_I2CM_XferHandler(adsDataX->i2cNumber, &i2cData);	//ejecuto el handler

			if(ret!=FALSE){
				adsDataX->comState = dataSet_state;
				ret = TRUE;
			}
			break;
		}
		default:
			ret = FALSE;
			break;
	}
	return ret;

	}

//Esta funcion lee un registro que le indico mediante regNumber en el ADS que le indico con la estructura adsDataX
//Si devuelve TRUE es que termino de leer
//si devuelve FALSE hay que volver a llamar a la funcion hasta que devuelva TRUE porque no termino
bool_t ads1115ReadRegister ( adsData_t *adsDataX,uint8_t regNumber )
{

	{
		I2CM_XFER_T i2cData;
		static uint32_t ret=0;
		// Armo el la estructura i2cData para pasarle al handler de comunicacion I2C
		switch (adsDataX->comState)
		{
			case dataSet_state:	//Enviar el primer byte, indica a que registro quiero acceder
			{
				i2cData.slaveAddr = adsDataX->i2cSlaveAddress;
				i2cData.options   = 0;
				i2cData.status    = 0;
				i2cData.txBuff    = &regNumber;
				i2cData.txSz      = 1;
				i2cData.rxBuff    = adsDataX->rxData;
				i2cData.rxSz      = 2;
					   Chip_I2CM_Xfer(adsDataX->i2cNumber, &i2cData); //le paso el puerto I2C que quiero escribir
																	   //y la informacion con la dreccion del esclavo
					   adsDataX->comState = sending_state;
					   ret = FALSE;
					   break;
			}
			case sending_state:
			{	//Ejecuto el handler hasta que se termine de transmitir
				if ( Chip_I2CM_StateChanged(adsDataX->i2cNumber) == 0)
					{
					ret = FALSE;
					break;
					}

								//si hay cambio en el controlador de I2C
				ret = Chip_I2CM_XferHandler(adsDataX->i2cNumber, &i2cData);	//ejecuto el handler
				if(ret!=FALSE)
				{
					adsDataX->comState = dataSet_state;
					ret = TRUE;
					break;
				}
				break;
			}
			default:
				ret = FALSE;
				break;
		}
		return ret;

		}
}
