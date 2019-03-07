all: LanciaServer LanciaClient
	gcc -o  LanciaServer ServerMess.c -Wall
	gcc -o LanciaClient ClientMess.c -Wall

LanciaClient: ClientMess.c
	gcc -o LanciaClient ClientMess.c -Wall
	

LanciaServer: ServerMess.c
	gcc -o LanciaServer ServerMess.c -Wall
	
Clean: 
	rm *o LanciaClient 	
