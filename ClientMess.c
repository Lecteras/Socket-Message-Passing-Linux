	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h> 
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <string.h>
	#include <stdio.h>
	#include <errno.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <string.h>
	#include <signal.h>
	#define BUFFER_SIZE 40
	#define BUF_LEN 2048
	#define TRUE 1
	#define FALSE 0
	#define CheckDim 11


typedef int BOOL;
void _register(int, char*,int,char*,char[]);
void who(int , char* );
void deregister(int, char*);
void quit (int, char*);
void _send(int, int);
void Gestore();

char UserName[BUFFER_SIZE];
char UserNameRegTemp[BUFFER_SIZE];
char Comando[BUFFER_SIZE];
char read_byte[CheckDim];
char send_byte[CheckDim];
int socketRecv;
int socketClient;
int optval=1;
char rispostaServer[BUF_LEN];
int ret, byte,n;
BOOL registered=FALSE;
BOOL isOffline=FALSE;
struct sockaddr_in server_addr, my_addr, ci_addr, cudp_addr, Rudp_addr;
//--------------------------------------------------------
//				MAIN
//--------------------------------------------------------

int main (int argc, char* argv[]){

	pid_t pid;

	char Comandi [BUF_LEN];
	int socketClient;
	int portaServer = atoi(argv[4]);

	int portaClientUDP = atoi(argv[2]);

	char *portaClientAscii= argv[2];//porta udp

	char* IpClient = argv[1];
	socketClient = socket(AF_INET, SOCK_STREAM, 0);
	socketRecv = socket(AF_INET,SOCK_DGRAM,0);
	setsockopt(socketRecv,  SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));//per riutilizzare le porte
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port=htons(portaServer);
	inet_pton(AF_INET,argv[3],&server_addr.sin_addr);



	//Struttura indirizzo client TCP
	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family=AF_INET;
	//Se impostato a 0 sceglie una porta qualunque libera (dalla guida di beej)
	my_addr.sin_port=0;
	int lenIp = strlen(IpClient);
	char IpClientAscii[lenIp];
	strcpy(IpClientAscii,IpClient);

	my_addr.sin_addr.s_addr = inet_addr(IpClientAscii);

	//Struttura indirizzo client UDP
	memset(&Rudp_addr,0,sizeof(Rudp_addr));
	Rudp_addr.sin_family=AF_INET;
		Rudp_addr.sin_port=htons(portaClientUDP);//vediamo se funziona
		inet_pton(AF_INET,IpClientAscii,&Rudp_addr.sin_addr);
		//	Rudp_addr.sin_addr.s_addr = INADDR_ANY;


	//Struttura indirizzo client per UDP //dove salvo i dati di chi mi scrive
		memset(&ci_addr,0,sizeof(ci_addr));

	//Struttura indirizzo client a cui voglio scrivere
		memset(&cudp_addr,0,sizeof(cudp_addr));
		cudp_addr.sin_family= AF_INET;



		ret=bind(socketClient,(struct sockaddr*)&my_addr,sizeof(my_addr));
		if(ret==-1){
			perror("Bind Error: ");
			exit(1);

		}

		ret=bind(socketRecv,(struct sockaddr*)&Rudp_addr,sizeof(Rudp_addr));
		if(ret==-1){
			perror("Bind2 Error: ");
			exit(1);
			
		}
		pid= fork();
		if(pid==0){
			char MessaggioIstantaneo[BUF_LEN];



			while(1){
				socklen_t lunCl=sizeof(ci_addr);

				ret = recvfrom(socketRecv, read_byte,CheckDim,0,(struct sockaddr*)&ci_addr, &lunCl);

				n= atoi(read_byte);

				ret= recvfrom(socketRecv, MessaggioIstantaneo,n, 0,(struct sockaddr*)&ci_addr, &lunCl);
				MessaggioIstantaneo[n]='\0';

				printf("%s\n",MessaggioIstantaneo);
				strcpy(MessaggioIstantaneo,"");

			}
			
		}else if(pid>0){

			ret = connect(socketClient, (struct sockaddr*)&server_addr ,sizeof(server_addr));

			if(ret==-1){
				perror("Connect Error: ");
			//	exit(1);
			}else {	
				printf("\nConnessione al server %s (porta %s) effettuata con successo.\n Ricezione messaggi istantanei su porta %s\n", argv[3],argv[4],argv[2]);
			}
//visualizzo i comandi
			ret = recv(socketClient, (void*)read_byte,CheckDim,0);
			n= atoi(read_byte);
			ret= recv(socketClient, (void*)Comandi,n,0);
			printf("%s\n",Comandi);

			while(1){
				if( (!strcmp(UserName," "))  ){
					printf("inserisci un comando> ");
				}else
				{
					printf("%s > ",UserName);
				}

				scanf("%s",Comando);

				switch(Comando[1]){
					case 'h':
					printf("%s\n",Comandi);	
					continue;
					case 'r':
					_register(socketClient,IpClientAscii,lenIp,portaClientAscii,rispostaServer );
					continue;
					case 'w':
					who(socketClient,  rispostaServer);
					continue;
					case  'q':
					quit(socketClient, rispostaServer);
					continue;
					case  'd':
					deregister(socketClient, rispostaServer);
					continue;	
					case   's':
					_send(socketClient, socketRecv);
					continue;		

				}

			}
		}else { 
			printf("Creazione fallita!\n");
			exit(1);
		}

	}
//------------------------------------------------------------------------------
//						FUNZIONI
//------------------------------------------------------------------------------
	void _register(int i,char* IpClientAscii,int lenIp, char* portaClientAscii,char*rispostaServer){
		if(registered==TRUE ){
			printf("sei già registrato, deregistrati per continuare!\n");
			
			return;
		}

		scanf("%s",UserNameRegTemp);

		if(strlen(UserNameRegTemp)>BUFFER_SIZE){
			printf("UserName Troppo lungo. Scegli un altro UserName\n");
			return;
		}
		if( (strcmp(UserNameRegTemp,UserName))!=0 && isOffline==TRUE ){
			printf("Sei offline, per registrarti nuovamente devi deregistrarti!\n");
			return;
		}else{
			isOffline=FALSE;
		}

		strcat(Comando,UserNameRegTemp);

		byte=strlen(Comando);
		Comando[byte]='\0';
		byte= strlen(Comando);
		sprintf(send_byte,"%d",byte);
		ret = send(i,(void*)send_byte,CheckDim,0 );
		
		ret = send(i, (void*)Comando, 2*byte, 0);
		

		ret = recv(i, (void*)read_byte,CheckDim,0);
		n= atoi(read_byte);
		
		char RispostaRegistrazione[n];
		strcpy(RispostaRegistrazione, " ");
		ret= recv(i, (void*)RispostaRegistrazione,n,0);
		RispostaRegistrazione[n]='\0';
		printf("%s\n",RispostaRegistrazione);
		registered=TRUE;
		strcpy(UserName,UserNameRegTemp);

		if(RispostaRegistrazione[0] !='E'&& RispostaRegistrazione[0]!='B' ){	
			byte= strlen(portaClientAscii);
			sprintf(send_byte,"%d",byte);
			ret = send(i,(void*)send_byte,CheckDim,0 );	
			ret = send(i,(void*)portaClientAscii,byte,0);
			byte= strlen(IpClientAscii);
			sprintf(send_byte,"%d",byte);
			ret = send(i,(void*)send_byte,CheckDim,0 );
			ret = send(i,(void*)IpClientAscii,byte,0);	
		}else if(RispostaRegistrazione[0]!='B'){
			registered=FALSE;
		}

		ret = recv(i, (void*)read_byte,CheckDim,0);
		n= atoi(read_byte);
		char MessaggioOffline[n];
		strcpy(MessaggioOffline, "");
		ret= recv(i, (void*)MessaggioOffline,n,0);
		MessaggioOffline[n]='\0';	
		printf("%s\n",MessaggioOffline);
		strcpy(Comando,"");
	}

	void who(int i, char* rispostaServer){

		if(registered==FALSE){
			printf("Devi prima registrarti!\n");
			return;
		}
		byte=strlen(Comando);
		Comando[byte]='\0';
		byte= strlen(Comando);
		sprintf(send_byte,"%d",byte);	
		ret = send(i,(void*)send_byte,CheckDim,0 );		
		ret = send(i, (void*)Comando, 2*byte, 0);

		ret = recv(i, (void*)read_byte,CheckDim,0);
		n= atoi(read_byte);
		char esitoWho[n];	
		ret= recv(i, (void*)esitoWho,n,0);
		esitoWho[n]='\0';
		printf("%s\n",esitoWho);
		strcpy(esitoWho,"");
		strcpy(Comando,"");


	}

	void deregister(int i, char* rispostaServer){
		if(registered==FALSE&&isOffline==FALSE){
			printf("Devi prima registrarti!\n");
			return;
		}
		registered=FALSE;
		isOffline=FALSE;

		strcat(Comando,UserName);
		byte=strlen(Comando);
		Comando[byte]='\0';
		sprintf(send_byte,"%d",byte);
		ret = send(i,(void*)send_byte,CheckDim,0 );
		ret = send(i, (void*)Comando, 2*byte, 0);

		ret = recv(i, (void*)read_byte,CheckDim,0);
		n= atoi(read_byte);
		strcpy(rispostaServer,"");
		char rispostaDeregistrazione[n];	
		ret= recv(i, (void*)rispostaDeregistrazione,n,0);
		rispostaDeregistrazione[n]='\0';	
		printf("%s\n",rispostaDeregistrazione);
		strcpy(UserName," ");
		strcpy(Comando,"");


	}

	void quit (int i, char* rispostaServer){
		if(registered==FALSE){
			printf("Devi prima registrarti!\n");
			return;
		}
		char esito[4]="";
		registered=FALSE;
		byte=strlen(Comando);
		Comando[byte]='\0';
		byte= strlen(Comando);
		sprintf(send_byte,"%d",byte);
		ret = send(i,(void*)send_byte,CheckDim,0 );
		ret = send(i, (void*)Comando, 2*byte, 0);

		byte= strlen(UserName);
		sprintf(send_byte,"%d",byte);
		ret = send(i,(void*)send_byte,CheckDim,0 );
		ret = send(i,(void*)UserName,byte,0);

		ret= recv(i, (void*)esito,4,0);
		if(strcmp(esito,"200")==0){
			printf("Sei stato disconnesso con successo\n");	
			isOffline=TRUE;
		//	close(socketRecv);
			exit(1);
		}else{
			printf("Non risulti registrato\n");											
		}
		strcpy(Comando,"");
	}

	void _send(int i, int j){
		char MessaggioDaInviare[BUF_LEN];
		strcpy(MessaggioDaInviare,"");

		char MessInvTemp[BUF_LEN]="";

		if(registered==FALSE){
			printf("Devi prima registrarti!\n");
			return;
		}
		
		char switchcase[2];
		char risposta[BUF_LEN];
		char UserNameUserNameDestinatario[BUFFER_SIZE];
		char PortaDestinatario[6];
		char IpDestinatario[BUFFER_SIZE];
		scanf("%s",UserNameUserNameDestinatario);

		if(strlen(UserNameUserNameDestinatario)>BUFFER_SIZE){
			printf("Username Destinatario Troppo lungo.\n");
			return;
		}
		
		int r = 0;

		strcpy(MessInvTemp," ");

		while((MessInvTemp[r] = getchar()) != '.') {
			r++;
		}
		r=strlen(MessInvTemp);

		MessInvTemp[r]='\0';

		strcat(MessaggioDaInviare,"Messaggio  da: ");
		strcat(MessaggioDaInviare,UserName);
		strcat(MessaggioDaInviare,"\n ");

		strcat(MessaggioDaInviare,MessInvTemp);
		byte= strlen(MessaggioDaInviare);
		MessaggioDaInviare[byte]='\0';
		if(strlen(MessaggioDaInviare)>BUF_LEN){
			printf("Messaggio troppo lungo.\n");
			return;
		}
		
		byte= strlen(Comando);
		sprintf(send_byte,"%d",byte);
		ret = send(i,(void*)send_byte,CheckDim,0 );
		ret = send(i, (void*)Comando, 2*byte, 0);

		byte= strlen(UserName);
		sprintf(send_byte,"%d",byte);
		
		ret = send(i,(void*)send_byte,CheckDim,0 );

		ret = send(i, (void*)UserName, byte, 0);	


		byte= strlen(UserNameUserNameDestinatario);
		sprintf(send_byte,"%d",byte);
		
		ret = send(i,(void*)send_byte,CheckDim,0 );
		ret = send(i, (void*)UserNameUserNameDestinatario, byte, 0);	
		
		ret = recv(i, (void*)read_byte,CheckDim,0);
		n= atoi(read_byte);
		ret= recv(i, (void*)risposta,n,0);
		risposta[ret]='\0';

		printf("%s\n",risposta);
		strcpy(risposta,"");
		ret= recv(i, (void*)switchcase,2,0);	
		strcpy(&switchcase[2],"\0");

		char decisione[2];

		switch(switchcase[1]){
								case 'F'://caso offline
								
								scanf("%s",decisione);

							if(!strcmp(decisione,"Y")){//se vuole lasciare un messaggio
								ret = send(i,(void*)decisione,2,0);
								byte= strlen(MessaggioDaInviare);
								if(byte>2048){
									printf("Non è possibile salvare messaggi cosi lunghi.");
									return;
								}
								sprintf(send_byte,"%d",byte);
								ret = send(i,(void*)send_byte,CheckDim,0 );
								ret = send(i,(void*)MessaggioDaInviare,byte,0);
								printf("perfetto, il messaggio verrà recapitato!\n");
							}else{//senon vuole lasciare un messaggio
								ret = send(i,(void*)decisione,2,0);
							}
							break;
								case 'N'://caso online

								scanf("%s",decisione);

							if(!strcmp(decisione,"Y")){//se vuole lasciare un messaggio

								ret = send(i,(void*)decisione,2,0);
								ret = recv(i, (void*)read_byte,CheckDim,0);
								n= atoi(read_byte);
								ret= recv(i,(void*)PortaDestinatario,n,0);

								ret = recv(i, (void*)read_byte,CheckDim,0);
								n= atoi(read_byte);
								ret= recv(i,(void*)IpDestinatario,n,0);
								
	//Inizializzo il client destinatario e lo contatto in udp

								int portadest=atoi(PortaDestinatario);
								cudp_addr.sin_port=htons(portadest);
								inet_pton(AF_INET,IpDestinatario,&cudp_addr.sin_addr);
								byte= strlen(MessaggioDaInviare);
								MessaggioDaInviare[byte]='\0';
								sprintf(send_byte,"%d",byte);

								ret = sendto(j,send_byte,CheckDim,0,(struct sockaddr*)&cudp_addr, sizeof(cudp_addr) );

								ret = sendto(j, MessaggioDaInviare, byte, 0,(struct sockaddr*)&cudp_addr, sizeof(cudp_addr));

								strcpy(MessaggioDaInviare,"");
								strcpy(MessInvTemp,"");


							}else{
								ret = send(i,(void*)decisione,2,0);
							}

							break;

							default:
							printf("\n");


						}
						strcpy(Comando,"");
					}




