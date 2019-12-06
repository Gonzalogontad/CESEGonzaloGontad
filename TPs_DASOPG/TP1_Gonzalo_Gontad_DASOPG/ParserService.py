#!/usr/bin/python3

import socket
import sys
import json
import csv
import time
import signal  
import traceback
import sys
import os


# Hacndler de captura de señales
def handler(sig, frame):  # define the handler  
    print("Signal Number:", sig, " Frame: ", frame)  
    traceback.print_stack(frame)	    #Imprimo el traceback de la señal capturada
    print('closing socket')     # Cierro el socket
    sock.close()
    print('Se presiono Ctrl+C!')
    sys.exit(0)     #Salir


#Configuro el Handler de la señal SIGINT
signal.signal(signal.SIGINT, handler)  # 

# Abro un socket UDP
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Leo el parametro que me pasan y lo tomo como puerto
port = 10000
try:
    port = int(sys.argv[1]) 
except:
    print("Puerto incorrecto") # si no hay puerto indico error 
    exit(1)
server_address = ('localhost', port) 
print('connecting to {} port {}'.format(server_address[0],server_address[1]))

# Me conecto al socket
sock.connect(server_address)


#Bucle principal
while True:
    
    #Abro el archivo config.txt cada vez que necesito leer el CSV por si cambio el Path

    with open(os.path.join(sys.path[0], "config.txt"), "r") as f: #Uso 'with' para no tener que cerrarlo usando 'try' y 'finally'
                                                                #sys.path[0] me da el path local
        file_name=str(f.read()) #Leo el path del archivo config.txt
        print(file_name)    
        file_name=file_name.rstrip("\n") #borro en \n del final
        
    #Abro el archivo CSV de las cotizaciones
    
    with open(file_name, mode='r') as csv_file: #Uso 'with' para no tener que cerrarlo usando 'try' y 'finally'

        curr = list(csv.DictReader(csv_file))   #Leo el CSV y lo guardo en una lista de OrederdDict
           
        convertedCurrList=list()    # Hago una nueva lista que contendra diccionarios con los keys que entiende PizarraService

        #   Recorro cada fila de la lista 'curr' tomando los datos correspondientes a 
        #   cada id y lo paso al diccionario con los id de PizarraService
        for row in curr:
            newCurrRow= dict.fromkeys(['id', 'value1', 'value2', 'name']) # Creo un diccionario con los Keys que entiende PizarraService
            
            currAux= dict(row)
            for key in currAux.keys():  #Recorro los Keys del diccionario
                
                if key == 'id':
                    newCurrRow['id']=int(currAux[key]) #tomo el valor y lo casteo segun el tipo de dato que corresponde al Key
                elif key == 'compra':
                    newCurrRow['value1']=float(currAux[key])
                elif key == 'venta':
                    newCurrRow['value2']=float(currAux[key])
                elif key == 'nombre':
                    newCurrRow['name']=currAux[key]
                else:
                    print("ERROR EN ARCHIVO")
            
            convertedCurrList.append(newCurrRow) #Agrego el diccionario a la lista que se va a enviar en formato JSON

        # Convierto la lista de diccionarios al formato JSON, codifico el mensaje en utf-8 y lo envio al socket UDP
        m = json.dumps(convertedCurrList) 
        message = m.encode("utf-8")
        sock.sendall(message)
        print(message)
        
        # Espero la respuesta de Pizarra service
        data = sock.recv(1024)
        print(type(data))
        print(str(data))

        # Espero 30 segundos antes de volver a actualizar los datos
        time.sleep(30)



        
