#include "transCode.h"


// Funcion que parsea un string proveniente de la UART, separa el valor de la linea a toglear
// y lo convierte en el formato aceptado por Interface Service
// Return =  cantidad de valores identificados correctamente, -1 cuando hay error

int traduceEduCiaaToIs(char* buffer)
{
    char line;
    int ret;
    // Hago el parseo del estado de las lineas en el paquete que recibo de Interface Service
    if (sscanf(buffer,">TOGGLE STATE:%c\r\n",&line)==1)
        {
        // Construyo el paquete de salida hacia la UART
        sprintf(buffer,":LINE%cTG\n",line);
        ret=1;
        }
    else ret=-1;
return (ret);
}

// Funcion que convierte la informacion recivida de Interface Service en el formato
// aceptado por la Edu-CIAA
// Return =  cantidad de valores identificados correctamente, -1 cuando hay error
int traduceIsToEduCiaa (char* buffer)
{
    char l1,l2,l3,l4;
    int ret;
    // Hago el parseo del estado de las lineas en el paquete que recibo de Interface Service
   if (sscanf(buffer,":STATES%c%c%c%c\n",&l1,&l2,&l3,&l4)==4)
    {
     // Construyo el paquete de salida hacia la UART
     sprintf(buffer,">OUTS:%c,%c,%c,%c\r\n",l1,l2,l3,l4);
     ret=4;
    }
    else ret=-1;
return (ret);
}
