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


#define BUF_LEN 2048
#define BUFFER_SIZE 40
#define TRUE 1
#define FALSE 0
#define LISTENER 10
#define dimIp 16
#define CheckDim 11

typedef int BOOL;

struct sockaddr_in my_addr;

//Struttura per salvare informazioni Client Online
typedef struct InfoClient{
	int id;
	char UsernameClient[BUFFER_SIZE];
	int PortaUDP;
	char ip[dimIp];
	BOOL Stato;//TRUE se online, FALSE altrimenti
	int PortaTcp;
}InfC;

//Struttura per sapvare Indirizzi Client
typedef struct IndirizziClient{
	struct sockaddr_in c_addr;
}IndC;

//struttura per salvare messaggi
typedef struct MessaggiSalvati{
	char UsernameC[BUFFER_SIZE];
	char MessaggioSalvato[BUF_LEN];
	char UsernameDest [BUFFER_SIZE];
}MessSaved;


void _register(int, InfC infoC[], IndC IC[], MessSaved MSv[]);
void deregister(int, InfC infoC[], IndC IC[]);
void who(int, InfC infoC[]);
void quit(int, InfC infoC[]);
void _send(int, InfC infoC[],MessSaved MSv[]);


char Comando[2*BUFFER_SIZE];
int NumClient=0;
char read_byte[CheckDim];
char send_byte[CheckDim];
int m=0; //indice messaggi salvati
int  i,j, byte,n;
BOOL Trovato=FALSE;
BOOL dereg=FALSE;
int ret;
int k; // memorizza temporaneamente NumClient alla riga 291

//struttura sockaddr per gestire disconnessione 

struct sockaddr_in ClientDisc_addr;

int main (int argc, char* argv[]){
	
	int porta= atoi(argv[1]);
	char comandi[BUF_LEN ]="\nSono disponibili i seguenti comandi:\n!help --> mostra l'elenco dei comandi disponibili\n!register username --> registrati!\n!deregister --> deregistrati!\n!who --> mostra l'elenco degli utenti disponibili ed il loro stato\n!send username --> inizia a chattare!\n!quit";
	
	socklen_t addrLen;	
	int listener;

	int newfd;	
	fd_set master;
	fd_set read_fds;	
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	int fdmax;
	

//alloco struttura informazioni client
	InfC *infoC;
	infoC=(InfC *) malloc(4*sizeof(InfC));
	
//alloco struttura indirizzi client		
	IndC *IC;
	IC = (IndC *) malloc(sizeof(IndC));
//alloco struttura messaggi client
	MessSaved *MSv;
	MSv = (MessSaved *) malloc(4*sizeof(MessSaved));
//creazione listener
	
	listener= socket(AF_INET,SOCK_STREAM,0);
	printf("Ho creato il socket di ascolto %d\n",listener);
	
//Struttura Indirizzo Server
	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(porta);
	my_addr.sin_addr.s_addr = INADDR_ANY; 

	
	ret=bind(listener,(struct sockaddr*)&my_addr,sizeof(my_addr));	
	if(ret==-1){
		perror("Bind Error: ");
		exit(1);
	}
	
	ret = listen(listener,LISTENER);	
	if(ret==-1){
		perror("Listen Error: ");
		exit(1);
	}
	
	
//inserisco il	 listener in master
	FD_SET(listener,&master);
//inizializzo il massimo descrittore di file
	fdmax=listener;
	
	while(1){
		
		read_fds = master;
		ret=select(fdmax+1, &read_fds,NULL,NULL,NULL);
		
		if(ret==-1){
			perror("Select Error: ");
			exit(1);
		}
		
		for(i=0; i<=fdmax; i++){
			if(FD_ISSET(i,&read_fds)){
				if(i == listener){
					addrLen = sizeof(IC[NumClient].c_addr);
					newfd = accept(listener,(struct sockaddr *)&IC[NumClient].c_addr, &addrLen);
					
					FD_SET(newfd, &master);
					if(newfd > fdmax){	
						fdmax = newfd; }
						
						printf("Nuova connessione sul socket: %d. ",newfd);
						printf("l'indirizzo del client è: %s.\n ", inet_ntoa(IC[NumClient].c_addr.sin_addr));

//invio i comandi
						
						byte= strlen(comandi);
						sprintf(send_byte,"%d",byte);
						ret = send(newfd,(void*)send_byte,CheckDim,0 );
						ret=send(newfd,(void*)comandi,byte,0);

					} else{
						
						strcpy(Comando,"");
						
						ret = recv(i, (void*)read_byte,CheckDim,0);
						n= atoi(read_byte);
						
						ret = recv(i,(void*)Comando,2*n,0);
						
						if(ret==0){
//gestito la chiusura inaspettata del terminale.
							printf("disconnessione non prevista\n");
							socklen_t addrClientLen=sizeof(ClientDisc_addr);
							//ret=getsockname( i, (struct sockaddr*)&ClientDisc_addr,&addrClientLen );
							ret=getpeername(i,(struct sockaddr*)&ClientDisc_addr,&addrClientLen );
							if(ret==-1){
								perror("getperrname Error: ");
							}
							char *IpDisconnesso;
							char NomeDisconnesso[BUFFER_SIZE];
							int PortaDisconessa;
							IpDisconnesso = inet_ntoa(ClientDisc_addr.sin_addr);
							PortaDisconessa=ntohs(ClientDisc_addr.sin_port);
							printf("Ip da disconnettere: %s\n",IpDisconnesso);
							printf("Porta da disconnettere: %d\n",PortaDisconessa );
							BOOL found=FALSE;
							for(j=0;j<=NumClient;j++){
								if(strcmp(IpDisconnesso, infoC[j].ip)==0 && (PortaDisconessa==infoC[j].PortaTcp) ){
									infoC[j].Stato=FALSE;
									found=TRUE;
									int LenD=strlen(infoC[j].UsernameClient);
									strncpy(NomeDisconnesso,infoC[j].UsernameClient,LenD);	
									NomeDisconnesso[LenD]='\0';
									break;			
								}	
							}
//gestisco il caso in cui un client prima di fare register chiuda il terminale
							if(found==FALSE){				
								printf("Un client non ancora registrato ha chiuso la connessione\n" );
									}else{
										printf("Il client %s si è disconnesso ed è stato messo offline.\n", NomeDisconnesso);
										found=FALSE;
									}
							close(i); 
							FD_CLR(i,&master);			
							
							strcpy(Comando,"n");
						}
						
//Guardo il comando ricevuto
						switch(Comando[1]){

							case 'r':
							_register(i, infoC, IC,MSv);
							continue;
							case 'w':
							who(i,infoC);
							continue;
							case  'q':
							quit(i,infoC);
							continue;
							case  'd':
							deregister(i, infoC, IC);
							continue;
							case   's':	
							_send(i,infoC,MSv);
							continue;
							default:
							
							continue;
							
						}
						
						close(i);
						FD_CLR(i,&master);			
					}
			}//chiudo if(FD_ISSET(i,&read_fds)){
			}//chiudo il for
			
		}//chiudo il while
	}//chiudo il main
	
	void _register(int i, InfC infoC[], IndC IC[], MessSaved MSv[]){
		char UserNameTemp[BUFFER_SIZE];	


		int socket = i;		
//prendo l'username dal comando							
		memcpy( UserNameTemp, &Comando[9], strlen(Comando)-9 );
		int DimUsername = strlen(Comando)-9;
		UserNameTemp[DimUsername] = '\0';
		BOOL existsMess=FALSE;


//Scorro la struttura per vedere se l'utente era già registrato ed online
		for(j=0;j<=NumClient;j++){
			if(strcmp(infoC[j].UsernameClient,UserNameTemp)==0 && infoC[j].Stato==TRUE){			
				Trovato=TRUE;
				char errore[27];
				strcpy(errore,"Errore! Utente già Online");
				errore[27]='\0';
				byte= strlen(errore);
				sprintf(send_byte,"%d",byte);
				ret = send(i,(void*)send_byte,CheckDim,0 );
				ret = send(i,(void*)errore,byte,0);
				printf("Errore! utente già online\n");
				break;	
			}		
		}
//Scorro la struttura per vedere se l'utente era già registrato ed offline	
		for(j=0;j<=NumClient;j++){
			if(strcmp(infoC[j].UsernameClient,UserNameTemp)==0 && infoC[j].Stato==FALSE){
				Trovato=TRUE;
				infoC[j].Stato=TRUE;
				char isBack[12];
				strcpy(isBack,"Bentornato!");
				isBack[12]='\0';
				byte= strlen(isBack);
				sprintf(send_byte,"%d",byte);
				ret = send(i,(void*)send_byte,CheckDim,0 );
				ret = send(i,(void*)isBack,byte,0);	
//aggiorno la porta tcp per una eventuale nuova disconnessione
				socklen_t addrClientLen=sizeof(ClientDisc_addr);

				ret=getpeername(i,(struct sockaddr*)&ClientDisc_addr,&addrClientLen );
				if(ret==-1){
					perror("getperrname Error: ");
				}
				int PortaTcp;
				PortaTcp=ntohs(ClientDisc_addr.sin_port);
				
				printf("Nuova Porta Tcp Usata: %d\n",PortaTcp );
				infoC[j].PortaTcp=PortaTcp;			
				break;			
			}
		}
		printf("Sto registrando L'userName: %s\n",UserNameTemp);

//prima di allocare una nuova istanza della struttura guardo se qualcuno si era deregistrato e uso quella posizione
		for(j=0;j<=NumClient;j++){
			if(infoC[j].id==0){
				dereg=TRUE;
				k=NumClient;
				NumClient=j;
				break;
			}	
		}
		
//se non era registrato lo inserisco
		if(Trovato==FALSE){
			char Benvenuto[48];
			strcpy(Benvenuto,"Registrazione avvenuta con successo, benvenuto!");
			Benvenuto[48]='\0';
			byte= strlen(Benvenuto);
			sprintf(send_byte,"%d",byte);
			ret = send(i,(void*)send_byte,CheckDim,0 );
			ret = send(i,(void*)Benvenuto,byte,0);
			
//inserisco un nuovo utente			
					strcpy(infoC[NumClient].UsernameClient,UserNameTemp);
			
//inizializzo lo stato
					infoC[NumClient].Stato=TRUE;	
//inizializzo l'id utente
					infoC[NumClient].id=NumClient+1;//per non farlo partire da 0. 0 lo assegno a chi si è deregistrato

//prendo la porta UDP del client e la metto nella struttura
					
					ret = recv(i, (void*)read_byte,CheckDim,0);
					n= atoi(read_byte);
					char PortaUdpTemp[n];
					ret=recv(i,(void*)PortaUdpTemp,n,0);					
					infoC[NumClient].PortaUDP=atoi(PortaUdpTemp);
					
//prendo l'indirizzo IP del Client e lo metto nella struttura

					ret = recv(i, (void*)read_byte,11,0);
					n= atoi(read_byte);
					char IPTemp [n];
					ret=recv(socket,(void*)IPTemp,byte,0);
					IPTemp[n]='\0';		
					char IpClient[n];					
					strcpy(IpClient,IPTemp);
					IpClient[n]='\0';
					strncpy(infoC[NumClient].ip,IpClient,n);
					strcpy(&infoC[NumClient].ip[n],"\0");
//prendo la porta Tcp usata (scelta casualmente)
					socklen_t addrClientLen=sizeof(ClientDisc_addr);

					ret=getpeername(i,(struct sockaddr*)&ClientDisc_addr,&addrClientLen );
					if(ret==-1){
						perror("getperrname Error: ");
					}
					int PortaTcp;
					PortaTcp=ntohs(ClientDisc_addr.sin_port);

					printf("Porta Tcp Usata: %d\n",PortaTcp );
					infoC[NumClient].PortaTcp=PortaTcp;
//ripristino NumCLient nel caso abbia usato una vecchia istanza di un utente che si era deregistrato
					if(dereg==TRUE){
						NumClient=k;
						dereg=FALSE;
					}
					NumClient++;//mi salvo il numero dei client nella struttura	
					printf("Utente %s registrato con successo\n",UserNameTemp);				
					}else{		
					Trovato=FALSE;
				}

//controllo se l'utente ha messaggi salvati
				int temp;
				for(j=0;j<=m;j++){
					if(strcmp(MSv[j].UsernameDest,UserNameTemp)==0 ){	
						existsMess=TRUE;
						temp=j;
						break;			
					}
				}
				if(existsMess){	
					char Segreteria[BUF_LEN];
					strcpy(Segreteria," ");
					strcat(Segreteria,"Ci sono dei messaggi (offline) per te!\n");
					//strcat(Segreteria,MSv[temp].UsernameC);
					//strcat(Segreteria,".\n");
					strcat(Segreteria,MSv[temp].MessaggioSalvato);
					byte= strlen(Segreteria);
					Segreteria[byte]='\0';
					sprintf(send_byte,"%d",byte);
					
					ret = send(i,(void*)send_byte,CheckDim,0 );
					ret = send(i,(void*)Segreteria,byte,0);					
//cancello la sua segreteria
					for(j=0;j<=m;j++){
						if(strcmp(MSv[j].UsernameDest,UserNameTemp)==0 ){
							
							strcpy(MSv[j].UsernameC,"");
							strcpy(MSv[j].MessaggioSalvato,"");
							strcpy(MSv[j].UsernameDest,"");	
							break;			
						}
					}		
				}else{
	char Segreteria[33];//se non ci sono messaggi la segreteria è vuota 
	strcpy(Segreteria,"Non hai nessun messaggio Offline");
	Segreteria[33]='\0';
	byte= strlen(Segreteria);
	sprintf(send_byte,"%d",byte);
	ret = send(i,(void*)send_byte,CheckDim,0 );
	ret = send(i,(void*)Segreteria,byte,0);
}
strcpy(UserNameTemp,"");

printf("Registrazione avvenuta con successo.\n");

}

void deregister(int i, InfC infoC[], IndC IC[]){

	char UserNameDerTemp[BUFFER_SIZE];
//prendo l'username dal comando							
	memcpy( UserNameDerTemp, &Comando[11], strlen(Comando)-11 );
	int DimUsername = strlen(Comando)-11;
	UserNameDerTemp[DimUsername] = '\0';
	printf("L'userName da deregistrare è: %s\n",UserNameDerTemp);
	
//Scorro la struttura per vedere se l'utente era già registrato	
	for(j=0;j<=NumClient;j++){
		if(strcmp(infoC[j].UsernameClient,UserNameDerTemp)==0 ){	

			Trovato=TRUE;
//reinizializzo la struttura
			infoC[j].id=0;//0 è un valore assegnato solo a chi si è deregistrato
			strcpy(infoC[j].UsernameClient," ");
			strcpy(infoC[j].ip," ");
			infoC[j].PortaUDP=0;
			//infoC[j].Stato==FALSE;
//Decremento il numero di client
			NumClient--;

//invio la risposta al clent;
			char result[50]="Deregistrazione avvenuta con successo, a presto!";
			result[50]='\0';
			byte= strlen(result);
			sprintf(send_byte,"%d",byte);
			ret = send(i,(void*)send_byte,CheckDim,0 );
			ret = send(i,(void*)result,byte,0);
			printf("Utente %s deregistrato\n", UserNameDerTemp);
			break;
			
		}

	}
			//se non era registrato mando un errore
	if(Trovato==FALSE){
		char errore[27]="Errore! Utente inesistente";
		errore[27]='\0';
		byte= strlen(errore);
		sprintf(send_byte,"%d",byte);
		ret = send(i,(void*)send_byte,CheckDim,0 );
		ret = send(i,(void*)errore,byte,0);
		printf("Errore! utente già online\n");

	}else{
		Trovato=FALSE;
	}

}

void who (int i, InfC infoC[]){
	printf("who\n");
	char WhoIsRegistered[BUF_LEN]= "Client registrati:\n";
	if(NumClient==0){
		strcat(WhoIsRegistered,"Nessuno è attualmente registrato");
	}else{
		for(j=0;j<=NumClient;j++){
			if(infoC[j].Stato==TRUE && infoC[j].id!=0){		
				strcat(WhoIsRegistered, infoC[j].UsernameClient);
				strcat(WhoIsRegistered, " (online)\n");
			}else if(infoC[j].id!=0 && infoC[j].Stato==FALSE ){//per non considerare eventuali deregistrazioni	
				strcat(WhoIsRegistered, infoC[j].UsernameClient);
				strcat(WhoIsRegistered, " (offline)\n");			
			}

		}
		
	}
	byte= strlen(WhoIsRegistered);
	sprintf(send_byte,"%d",byte);
	ret = send(i,(void*)send_byte,CheckDim,0 );	
	ret = send(i,(void*)WhoIsRegistered,byte,0);
	printf("who is registered %s\n",WhoIsRegistered );
	if(ret==-1){
		perror("Send Error: ");
		exit(1);
	}
	
	
	
	for(j=0;j<=NumClient;j++){
		if(infoC[j].id<10000 && infoC[j].id!=0 ){
			printf("id: %d, Username: %s, PortaUDP: %d, IP: %s, Stato: %d , Porta Tcp: %d\n",infoC[j].id,infoC[j].UsernameClient,infoC[j].PortaUDP,infoC[j].ip,infoC[j].Stato,infoC[j].PortaTcp);
		}
	}
	printf("Fine Who\n");
}


void quit(int i, InfC infoC[]){
	printf("Quit\n");

	char UserDaDisconnettere [BUF_LEN]=" ";
	BOOL exists= FALSE;
	ret = recv(i, (void*)read_byte,CheckDim,0);
	n= atoi(read_byte);
	ret = recv(i,(void*)UserDaDisconnettere,n,0);
	UserDaDisconnettere[n]='\0';
	printf("Il client da rendere offline è: %s\n",UserDaDisconnettere);


	for(j=0;j<=NumClient;j++){
		if(strcmp(UserDaDisconnettere, infoC[j].UsernameClient)==0 ){
			infoC[j].Stato=FALSE;
			exists=TRUE;
			break;
			
		}	
	}
	char ok[4];
	if(exists==TRUE){
		strcpy(ok,"200");
		ret = send(i,(void*)ok,4,0);
		printf("Client messo offline con successo.\n");
	}else{
		strcpy(ok,"400");
		ret = send(i,(void*)ok,4,0);
		printf("Client Non trovato\n");
	}
	
	
}

void _send(int i, InfC infoC[],  MessSaved MSv[]){
	
	char UserNameMittente[BUFFER_SIZE];
	char UserNameDestinatario[BUFFER_SIZE];
	BOOL isOnline = FALSE;
	BOOL ReceiverExists = FALSE;
	char result[BUF_LEN];
	strcpy(result," ");
	int id;

	char switchcase[2];//I inesistente, ON online, OF offline
	char MessaggioDaInviare[BUF_LEN];
	char IpDestinatario[BUFFER_SIZE];
	char porta[6];
//ricevo l'username del mittente
	ret = recv(i, (void*)read_byte,CheckDim,0);
	n= atoi(read_byte);
	
	ret=recv(i, (void*) UserNameMittente,n,0);
	UserNameMittente[n]='\0';
	printf("l'UserNameMittente è: %s\n",UserNameMittente );

	if(ret==-1){
		perror("Errore:");
	}
//ricevo l'username del destinatario
	ret = recv(i, (void*)read_byte,CheckDim,0);
	n= atoi(read_byte);

	ret= recv(i, (void*) UserNameDestinatario,n,0);
	UserNameDestinatario[n]='\0';
	printf("l' UserNameDestinatario è: %s\n",UserNameDestinatario );

	if(ret==-1){
		perror("Errore:");
	}
//cerco il destinatario e guardo se è online
	for(j=0;j<=NumClient;j++){
		if(strcmp(UserNameDestinatario, infoC[j].UsernameClient)==0 ){
			if(infoC[j].Stato==TRUE){
				isOnline=TRUE;
				id=j;
				
				
			}				
			ReceiverExists=TRUE;
			
			break;
			
		}	
	}
	
	if(ReceiverExists==TRUE){
		if(isOnline==TRUE){
			strcpy(switchcase, "ON");
			strcat(result,"l'utente è online, la sua porta è: ");
			
			sprintf(porta, "%d", infoC[id].PortaUDP);
			strcat(result,porta);
			strcat(result," ed il suo ip è: ");
			strcat(result,infoC[id].ip);
			strcpy(IpDestinatario,infoC[id].ip);
			strcat(result," vuoi contattarlo? (Y/N) ");
			printf("Chiedo all'utente cosa vuole fare\n");	
		}else{
			strcpy(switchcase, "OFF");
			strcat(result,"l'utente è 'offline', vuoi che il messaggio venga consegnato appena possibile?(Y/N)");
		}
	}else{
		strcpy(switchcase, "I");
		strcat(result,"Impossibile connettersi a username: utente inesistente.");

	}


	byte= strlen(result);
	sprintf(send_byte,"%d",byte);
	ret = send(i,(void*)send_byte,CheckDim,0 );
	
	ret = send(i,(void*)result,byte,0);
	strcpy(result,"");
	if(ret==-1){
		perror("Errore:");
	}

	ret = send(i,(void*)switchcase,2,0);
	if(ret==-1){
		perror("Errore:");
	}

	strcpy(&switchcase[2],"\0");
	printf("l'utente richiesto è: %s\n",switchcase);
	char decisione[2];
	switch(switchcase[1]){
							case 'F'://caso offline
							printf("Destinatario Offline\n");
							
							ret=recv(i,(void*)decisione,2,0);
								if(!strcmp(decisione,"Y")){//se vuole lasciare un messaggio
									ret = recv(i, (void*)read_byte,CheckDim,0);
									n= atoi(read_byte);
									ret=recv(i,(void*)MessaggioDaInviare,n,0);
									MessaggioDaInviare[n]='\0';
									
									printf("Il mess da inviare offline è: %s\n",MessaggioDaInviare);
									//printf("m vale %d:",m);
									m++;m++;
									strcpy(MSv[m].MessaggioSalvato,MessaggioDaInviare);
									strcpy(MSv[m].UsernameC,UserNameMittente);
									strcpy(MSv[m].UsernameDest,UserNameDestinatario);
									
									
									return;
								}else{
									//non vuole lasciare un messaggio
									printf("L'utente %s non vuole lasciare un mess\n", UserNameMittente);
									
								}
								break;
								
							case 'N'://caso online
							printf("Destinatario Online\n");
							ret=recv(i,(void*)decisione,2,0);
								if(!strcmp(decisione,"Y")){//se vuole avere i dati
									printf("Invio Ip e PortaUDP\n");
									byte= strlen(porta);
									sprintf(send_byte,"%d",byte);
									ret = send(i,(void*)send_byte,CheckDim,0 );
									ret= send(i,(void*)porta,byte,0);

									byte= strlen(IpDestinatario);
									sprintf(send_byte,"%d",byte);
									ret = send(i,(void*)send_byte,CheckDim,0 );
									ret= send(i,(void*)IpDestinatario,byte,0);
									printf("Ip e PortaUDP inviati con successo.\n");
								}else{
									//non vuole avere i dati
									printf("L'utente %s non vuole contattare il destinatario\n", UserNameMittente);
									
								}
								break;
								
								default:
								printf(" \n");
								
								
							}
							


						}

						

