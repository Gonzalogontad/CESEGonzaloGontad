#include <sapi.h>

#include <MCP4725.h>

bool_t mcp4725NormalSend(mcpData_t *mcpDataX) //le paso un puntero a la estructura que contiene todos los datos de un ADS
{
	I2CM_XFER_T i2cData;		//eliminar las variables locales para que se pueda instanciar varias veces la funcion
	uint8_t txBuffer[3];
	static uint32_t ret = 0;
	//mcpDataX->txData=((mcpDataX->txData)<<4);
	// Prepare the i2cData register

	switch (mcpDataX->comState)
	{
	case dataSet_state:	//Enviar el primer byte, indica a que registro quiero acceder
	{

		txBuffer[0] = mcpDataX->mode; //escribir el registro de configuracion
		txBuffer[1] = (uint8_t)(((mcpDataX->txData)>>4)& (0x00FF)); 		//le paso la parte alta
		txBuffer[2] = (uint8_t)(((mcpDataX->txData)<<4)& (0x00F0));		//le paso la parte baja

		i2cData.slaveAddr = mcpDataX->i2cSlaveAddress;
		i2cData.options = 0;
		i2cData.status = 0;
		i2cData.txBuff = txBuffer;
		i2cData.txSz = 3;
		i2cData.rxBuff = 0;
		i2cData.rxSz = 0;

		Chip_I2CM_Xfer( mcpDataX->i2cNumber, &i2cData); //le paso el puerto I2C que quiero escribir
											//y la informacion con la dreccion del esclavo
		mcpDataX->comState = sending_state;
		break;
	}
	case sending_state:
	{	//Ejecuto el handler hasta que se termine de transmitir
		if (Chip_I2CM_StateChanged(mcpDataX->i2cNumber) == 0)
		{
			return FALSE;
		}

		//si hay cambio en el controlador de I2C
		ret = Chip_I2CM_XferHandler(mcpDataX->i2cNumber, &i2cData);	//ejecuto el handler de enviar
		if (ret == FALSE)
		{
			return FALSE;
			break;
		}
		else
		{
			mcpDataX->comState = dataSet_state;
			return TRUE;
		}

		break;
	}
	default:
		break;
	}
	return FALSE;

}



