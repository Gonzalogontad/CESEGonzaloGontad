
#include <stdio.h>
#include <unistd.h>
#include "SerialManager.h"
#include "transCode.h"
#include <string.h>
#define BUFFER_SIZE 128

void* toTcpClient (void*);
