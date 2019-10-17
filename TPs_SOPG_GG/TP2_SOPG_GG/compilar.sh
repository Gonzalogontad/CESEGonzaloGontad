gcc server_tcp.c ClientData.c SerialManager.c rs232.c -pthread -o server
gcc cliente_tcp.c -o client
