#include <sapi.h>

#include <MCP4725.h>

bool_t mcp4725NormalSend(mcpData_t *mcpDataX) //le paso un puntero a la estructura que contiene todos los datos de un mcp
{
	I2CM_XFER_T i2cData;
	uint8_t txBuffer[3];
	static uint32_t ret = 0;


	switch (mcpDataX->comState)
	{
	case dataSet_state:	//Enviar el primer byte, indica a que registro quiero acceder
	{

		txBuffer[0] = (mcpDataX->pdMode|WRITE_DAC_REG); //escribir el registro de configuracion

		//Los datos en modo normal para el MCP4725 (12bits) se transmiten primero los 8 bits mas significativos
		//y luego los 4 menos significativos. Los ultimos 4 bits de la trama no se usan (los pongo en cero)

		txBuffer[1] = (uint8_t)(((mcpDataX->txData)>>4)& (0x00FF)); 		//le paso la parte alta
		txBuffer[2] = (uint8_t)(((mcpDataX->txData)<<4)& (0x00F0));		//le paso la parte baja

		//Armo la estructura i2cData que le voy a pasar al handler de comunicacion del bus I2C
		i2cData.slaveAddr = mcpDataX->i2cSlaveAddress; //direccion del MCP4725
		i2cData.options = 0;
		i2cData.status = 0;
		i2cData.txBuff = txBuffer; //puntero al buffer de datos a enviar
		i2cData.txSz = 3;	//cantidad de bytes a enviar
		i2cData.rxBuff = 0;
		i2cData.rxSz = 0;

		Chip_I2CM_Xfer( mcpDataX->i2cNumber, &i2cData); //le paso el puerto I2C que quiero escribir
														//y la informacion con la dreccion del esclavo
		mcpDataX->comState = sending_state;
		ret=0;
		break;
	}
	case sending_state:
	{
		if (Chip_I2CM_StateChanged(mcpDataX->i2cNumber) == 0)//Veo si hay cambio en el controlador de I2C
		{
			ret = FALSE;
			break;
		}

		ret = Chip_I2CM_XferHandler(mcpDataX->i2cNumber, &i2cData);	//ejecuto el handler de comunicacion I2C
		if (ret != FALSE)	//si devuelve distinto de false significa que ya termino de enviar.
		{
			mcpDataX->comState = dataSet_state;
			ret = TRUE;		//devuelvo TRUE indicando que ya se termino la comunicacion
			break;
		}

		ret = FALSE;
		break;
	}
	default:
		ret = FALSE;
		break;
	}
	return ret;

}

bool_t mcp4725FastSend(mcpData_t *mcpDataX) //le paso un puntero a la estructura que contiene todos los datos de un mcp
{
	I2CM_XFER_T i2cData;
	uint8_t txBuffer[3];
	static uint32_t ret = 0;

	switch (mcpDataX->comState)
	{
	case dataSet_state:	//Enviar el primer byte, indica a que registro quiero acceder
	{


		//Los datos en modo FAST para el MCP4725 (12bits) se transmiten primero los 2 bits de modo fast, luego 2 bits
		//del modo Power Down y luego los 12 bits del dato

		txBuffer[0] = (uint8_t)(((mcpDataX->txData)>>8)& (0x00FF))|mcpDataX->pdMode|FASTE_WR_DAC_REG; //le paso la parte alta
		txBuffer[1] = (uint8_t)((mcpDataX->txData)& (0x00FF));		//le paso la parte baja

		//Armo la estructura i2cData que le voy a pasar al handler de comunicacion del bus I2C
		i2cData.slaveAddr = mcpDataX->i2cSlaveAddress; //direccion del MCP4725
		i2cData.options = 0;
		i2cData.status = 0;
		i2cData.txBuff = txBuffer; //puntero al buffer de datos a enviar
		i2cData.txSz = 2;	//cantidad de bytes a enviar
		i2cData.rxBuff = 0;
		i2cData.rxSz = 0;

		Chip_I2CM_Xfer( mcpDataX->i2cNumber, &i2cData); //le paso el puerto I2C que quiero escribir
														//y la informacion con la dreccion del esclavo
		mcpDataX->comState = sending_state;
		ret=0;
		break;
	}
	case sending_state:
	{
		if (Chip_I2CM_StateChanged(mcpDataX->i2cNumber) == 0)//Veo si hay cambio en el controlador de I2C
		{
			ret = FALSE;
			break;
		}

		ret = Chip_I2CM_XferHandler(mcpDataX->i2cNumber, &i2cData);	//ejecuto el handler de comunicacion I2C
		if (ret != FALSE)	//si devuelve distinto de false significa que ya termino de enviar.
		{
			mcpDataX->comState = dataSet_state;
			ret = TRUE;		//devuelvo TRUE indicando que ya se termino la comunicacion
			break;
		}

		ret = FALSE;
		break;
	}
	default:
		ret = FALSE;
		break;
	}
	return ret;

}


