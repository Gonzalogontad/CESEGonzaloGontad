#include <sapi.h>

#include <ADS1115.h>



bool_t ads1115Init ( adsData_t *adsDataX) //le paso un puntero a la estructura que contiene todos los datos de un ADS
	{

	I2CM_XFER_T i2cData;
	uint8_t txBuffer[3];
	static uint32_t ret=0;
	// Prepare the i2cData register


	switch (adsDataX->comState)
	{
		case dataSet_state:	//Enviar el primer byte, indica a que registro quiero acceder
		{
			txBuffer[0] = (uint8_t)configReg; 	//escribir el registro de configuracion
			txBuffer[1] = adsDataX->modeHi; 		//le paso la parte alta
			txBuffer[2] = adsDataX->modeLo;		//le paso la parte baja
			i2cData.slaveAddr = adsDataX->i2cSlaveAddress;
			i2cData.options   = 0;
			i2cData.status    = 0;
			i2cData.txBuff    = txBuffer;
			i2cData.txSz      = 6;
			i2cData.rxBuff    = 0;
			i2cData.rxSz      = 0;


				   Chip_I2CM_Xfer(adsDataX->i2cNumber, &i2cData); //le paso el puerto I2C que quiero escribir
																   //y la informacion con la dreccion del esclavo
				   adsDataX->comState = sending_state;
				   break;
		}
		case sending_state:
		{	//Ejecuto el handler hasta que se termine de transmitir
			if ( Chip_I2CM_StateChanged(adsDataX->i2cNumber) == 0)
			{
				return FALSE;
			}

			//si hay cambio en el controlador de I2C
			ret = Chip_I2CM_XferHandler(adsDataX->i2cNumber, &i2cData);	//ejecuto el handler
			//ret = Chip_I2CM_XferHandler(adsDataX->i2cNumber, &i2cData);	//ejecuto el handler
			if(ret==FALSE){
				return FALSE;
				break;}
			else
			{
				adsDataX->comState = dataSet_state;
				return TRUE;
			}

			break;
		}
		default:
			break;
	}
	return FALSE;

	}

bool_t ads1115Read ( adsData_t *adsDataX)
{

	{
		I2CM_XFER_T i2cData;
		uint8_t txBuffer;
		static uint32_t ret=0;
		// Prepare the i2cData register

		switch (adsDataX->comState)
		{
			case dataSet_state:	//Enviar el primer byte, indica a que registro quiero acceder
			{
				txBuffer = (uint8_t)convertionReg; 	//leer el registro de conversion
				i2cData.slaveAddr = adsDataX->i2cSlaveAddress;
				i2cData.options   = 0;
				i2cData.status    = 0;
				i2cData.txBuff    = &txBuffer;
				i2cData.txSz      = 1;
				i2cData.rxBuff    = adsDataX->rxData;
				i2cData.rxSz      = 2;
					   Chip_I2CM_Xfer(adsDataX->i2cNumber, &i2cData); //le paso el puerto I2C que quiero escribir
																	   //y la informacion con la dreccion del esclavo
					   adsDataX->comState = sending_state;
					   break;
			}
			case sending_state:
			{	//Ejecuto el handler hasta que se termine de transmitir
				if ( Chip_I2CM_StateChanged(adsDataX->i2cNumber) == 0)
					{
								return FALSE;
							}

								//si hay cambio en el controlador de I2C
				ret = Chip_I2CM_XferHandler(adsDataX->i2cNumber, &i2cData);	//ejecuto el handler
				if(ret==0)
					break;
				else
				{
					adsDataX->comState = dataSet_state;
					return TRUE;
				}

				break;
			}
			default:
				break;
		}
		return FALSE;

		}
}
