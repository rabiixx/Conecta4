/* Librerias */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <strings.h>		
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "conecta4_bot.h"

/* Constantes */ 
#define MAXDATASIZE 2048	/* Tamano del buffer */
#define MAXNAMESIZE 16		/* Maximo tamano del nombre */
#define MAXUSERSIZE 12		/* Maximo tamano del nombre de usuario */
#define MAX_PARTIDAS 4		/* Numero maximo de partidas */
#define TRUE 1
#define FALSE 2


/* Informacion sobre jugador */
typedef struct _jugador{
	int fd;						/* Descriptor del socket cliente */
	FILE *f;					/* FILE del cliente para la comunicacion a mas alto nivel */
	char *nombre;				/* Nobre del cliente */
	char *user_id;				/* id del usuario */
	int res;					/* Resultado de la operacion, enviada por el cliente */
	int player;					/* Ficha del jugador */
}jugador;

/* Informacion sobre la partida */
typedef struct _partida{
	jugador Jugador;						/* Array de los jugadores que jugaran la partidas */
	char **tablero;							/* Tablero de la partida */
	int START_FLAG;							/* Indica que la partida esta lista para comenzar */
	int PLAYING_FLAG;						/* Indica que la partida esta en juego */
	int END_FLAG;							/* Indica que la partidas ha finlizado */
	int numJugadores;						/* Indica el numero de jugadores conectados a la partida */
}partida;

typedef struct  _bot{
	char name[16];
	char player;
}bot;

/* Variables globales */
int numPartidas = 0;				/* Numero de partidas que se estan jugandose */
int server_socketfd;				/* Descriptor de fichero de socket servidor */
partida arrPartidas[MAX_PARTIDAS];	/* Array con los datos de cada partida */

/* Definicion de funciones */
void salir_correctamente(int code);

int connect4(int maxRow, int maxCol, int **tablero, int player);

int meterFicha(int nCol, int nFil, int **matrix, int col, int player);

int max(int, int);

void clearPartida(int);

void compactaClavesP(void);


/* Declaracion de funciones */
char *randString(){

    char *str = (char*)calloc(100, sizeof(char));
    srand(time(NULL));
    size_t size = (rand() % (12 - 6 + 1)) + 6;//rand entre 6 y 12

    const char charset1[] = "abcdefghijklmnopqrstuvwxyz";
    const char charset2[] = "0123456789";

    for (int n = 0; n < size; n++) {
        if ( rand() % 2 == 1){
            int i = rand() % 24;
            str[n] = charset1[i];
        }else{
            int i = rand() % 9;
            str[n] = charset2[i];
        }

    }
    str[size] = '\0';
    return str;
}

/* Genera un numero de tipo entero de forma aleatoria */
int random_number(int xi, int xm){
	int i, count = 5, upper = xm;
	int num;
	int  lower = xi;
	srand(time(NULL));
    for (i = xi; i < count; i++) { 
		num = (rand() % (upper - lower + 1)) + lower; 
    } 
    return num;
}

void handle_signal_action(int sig_number){
  	if (sig_number == SIGINT) {
    	printf("SIGINT was catched!\n");
    	salir_correctamente(EXIT_SUCCESS);
  	}
  	else if (sig_number == SIGPIPE) {
    	printf("SIGPIPE was catched!\n");
    	salir_correctamente(EXIT_SUCCESS);
  	}
}

int setup_signals(){
  	struct sigaction sa;
  	sa.sa_handler = handle_signal_action;
  	if (sigaction(SIGINT, &sa, 0) != 0) {
    	perror("sigaction()");
    	return -1;
  	}
  	if (sigaction(SIGPIPE, &sa, 0) != 0) {
    	perror("sigaction()");
    	return -1;
  	}
  return 0;
}

void salir_correctamente(int code){

	for (int i = 0; i < numPartidas; ++i){
		fclose(arrPartidas[i].Jugadores[0].f);
		fclose(arrPartidas[i].Jugadores[1].f);
		close(arrPartidas[i].Jugadores[0].fd);
		close(arrPartidas[i].Jugadores[1].fd);
	}

  	close(server_socketfd);

	printf("Shutdown server properly.\n");
	exit(code);
}

void protocolError(FILE *f, int fd){

	fclose(f);
	close(fd);
}


int main(int argc, char const *argv[]){
	
	system("clear");
	
	if (argc != 4){
        perror("Usage: ./servidor <puerto> <numero_filas> <numero_columnas>");
        exit(EXIT_FAILURE);
    }

    if (setup_signals() != 0){
    	exit(EXIT_FAILURE);
    }

	/* Variables */ 
	int nFilas = atoi(argv[2]);
	int nColumnas = atoi(argv[3]);

	/* Variables Sockets */
    int 				client_socketfd;			/* Descriptor de fichero de socket cliente */ 		
    struct sockaddr_in  server_dir;					/* Informacion sobre servidor */
    struct sockaddr_in  client_dir;					/* Informacion sobre direccion del cliente */
    socklen_t 			addrlen;					/* Tamaño de la informacion del cliente */
    char 				buffer[MAXDATASIZE];		/* Buffer General de 1024*sizeof(char) */
	int 				Puerto = atoi(argv[1]);		/* Puerto de escucha */    
	int running = 1;								/* Servidor en funcionamiento */
	FILE *usersDB = NULL;							/* Base de datos: Fichero con nombre de usuario, resultado y nombre de todos los usuarios */
	int TIE_FLAG = -1;
	bot Bot;
	strcpy(Bot.name, "BOT-FLEXX");
	Bot.player = 'X';
    /* Select */
	fd_set readfds;						/* Descriptores de interes para select() */
	fd_set except_fds;								/* Control de excepciones */
	int maximo;										/* Número de descriptor más grande */						
	/*struct timeval selTimeout;*/						/* Timeout for select() */
	//time_t server_start_t, end_t;					/* Tiempo de inicial, tiempo actual/final */
	//time(&server_start_t);							/* Tiempo en el que el servidopr comienza su ejecucion */

	/* Create socket */
    if( (server_socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    /* Server information */
    server_dir.sin_family         = AF_INET;
    server_dir.sin_port           = htons(Puerto);
    server_dir.sin_addr.s_addr    = INADDR_ANY;
    memset(&(server_dir.sin_zero),'\0', 8);	/* Ponemos a cero el resto de la estructura */
    
    /* Call bind */ 
    if ( bind(server_socketfd, (struct sockaddr *)&server_dir, sizeof(server_dir)) == -1){
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
	/* Listen */
	if ( listen(server_socketfd, 10) != -1){
		printf("[+] Escuchando conexiones en el puerto %d\n", Puerto);
	}else{
		printf("[+] Nueva conexión denegada, número máximo de clientes alcanzado\n");
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    /* Servidor activo */
	while (running){

		/* Comprobamos si hay alguna partida para comenzar 
		* o si bien hay alguna partida que haya finalizado */
H:		for (int i = 0; i < numPartidas; ++i){

			/* Comienzo de la partida */
			if (arrPartidas[i].START_FLAG == TRUE){
				
				fprintf(arrPartidas[i].Jugador.f, "START %s %d %d\n", arrPartidas[i].Jugador.nombre, nFilas, nColumnas);
				
				/* Quien empieza jugando ? */
				time_t t;
				srand((unsigned) time(&t));
				bool x = rand() & 1;
				if(x){
					fprintf(arrPartidas[i].Jugador.f, "URTURN\n");
					printf("[+] %s empiezas jugando.\n", arrPartidas[i].Jugadores[0].nombre);					
				}else{
					printf("[+] Empiezas jugando %s.\n", );
				}

				arrPartidas[i].Jugador.player = 'O';
				printf("[+] Simbolo de %s = %d\n", arrPartidas[i].Jugador.nombre, arrPartidas[i].Jugador.player);
				printf("[+] Simbolo de %s = %d\n", Bot.name, Bot.player);

				for (int z = 0; z < nFilas; ++z)
					for (int j = 0; j < nColumnas; ++j)
						arrPartidas[i].tablero[z][j] = '-';

				arrPartidas[i].START_FLAG = FALSE;	
				arrPartidas[i].PLAYING_FLAG = TRUE;
			}
		}

		compactaClavesP();

		printf("[+] Numero de partidas en juego: %d\n", numPartidas);
		for (int i = 0; i < numPartidas; ++i) {
			printf("\t- Partida %d: %s VS %s\n", i, Bot.name, arrPartidas[i].Jugador.nombre);
		}
	
		FD_ZERO (&readfds);
		FD_SET (server_socketfd, &readfds);
		FD_ZERO (&except_fds);
		FD_SET (server_socketfd, &except_fds);

		for (int i = 0; i < numPartidas; ++i) {
			FD_SET(arrPartidas[i].Jugador.fd, &readfds);
			FD_SET(arrPartidas[i].Jugador.fd, &except_fds);
			maximo = max(maximo, arrPartidas[i].Jugador.fd);
		}

		maximo = max(server_socketfd, maximo);

		int activity = select (maximo + 1, &readfds, NULL, &except_fds, NULL);
	    
	    switch (activity) {
	      	case -1:
	        	perror("select()");
	        	salir_correctamente(EXIT_FAILURE);

	      	case 0:
	       		/*for (int i = 0; i < numClientes; ++i){
					time(&end_t);
					printf("[+] Enviando información de tiempo a %s\n", jugadores[i].user_id);
					fprintf(jugadores[i].f, "UPTIME %f %f\n", difftime(end_t, server_start_t), difftime(end_t, jugadores[i].client_start_t));
				}*/
	    	default:

				for (int i = 0; i < numPartidas; i++){
					for (int j = 0; j < MAX_JUGADORESP; ++j){
						if (FD_ISSET(arrPartidas[i].Jugadores[j].fd, &readfds) ){
							
							char cmd[MAXDATASIZE];
							int tempColumna;
							if (fgets(buffer, MAXDATASIZE, arrPartidas[i].Jugadores[j].f) == NULL){
								perror("fgets failed");
								salir_correctamente(EXIT_FAILURE);
							}

							sscanf(buffer, "%s", cmd);
							if (strcmp("COLUMN", cmd) == 0) {
								sscanf(buffer, "%*s %d", &tempColumna);
								if (meterFicha(nColumnas, nColumnas,  arrPartidas[i].tablero, tempColumna, arrPartidas[i].Jugador.player) == -1){
									fprintf(arrPartidas[i].Jugador.f, "COLUMN ERROR\n");
								} else {

									fprintf(arrPartidas[i].Jugadores[j].f, "COLUMN OK\n");	

									/* Comprobacion empate */
									for (int z = 0; z < nFilas; ++z)
							            for (int j = 0; j < nColumnas; ++j){
							                if (arrPartidas[i].tablero[z][j] == '-'){
							                    TIE_FLAG = FALSE;
							                    break;
							                }
							            }

							        printf("[+] Partida %d: %s VS %s\n", i, Bot.name arrPartidas[i].Jugador.nombre);
							        for (int z = 0; z < nFilas; ++z){
								    	for (int j = 0; j < nColumnas; ++j){
								    		printf("| %c |\t", arrPartidas[i].tablero[z][j]);
								    	}
								    	printf("\n");
								    }
								    printf("\n\n");

									if (connect4(nFilas, nColumnas, arrPartidas[i].tablero, arrPartidas[i].Jugadores[j].player) == TRUE){
										if (j == 0) {
											fprintf(arrPartidas[i].Jugadores[j].f, "VICTORY\n");
											fprintf(arrPartidas[i].Jugadores[j + 1].f, "DEFEAT\n");
											printf("[+] La partida %d ha finalizado\n", i);
											clearPartida(i);
										} else if(j == 1) {
											fprintf(arrPartidas[i].Jugadores[j].f, "VICTORY\n");
											fprintf(arrPartidas[i].Jugadores[j - 1].f, "DEFEAT\n");
											printf("[+] La partida %d ha finalizado\n", i);
											clearPartida(i);
										}
									} else if (TIE_FLAG == TRUE) {
										fprintf(arrPartidas[i].Jugadores[0].f, "TIE\n");
										fprintf(arrPartidas[i].Jugadores[1].f, "TIE\n");
										printf("[+] La partida %d ha finalizado\n", i);
										clearPartida(i);
									}

									if (j == 0) {
										fprintf(arrPartidas[i].Jugadores[j + 1].f, "URTURN %d\n", tempColumna);
										printf("[+] Es el turno de %s.\n", arrPartidas[i].Jugadores[j + 1].nombre);
									} else if (j == 1) {
										fprintf(arrPartidas[i].Jugadores[j - 1].f, "URTURN %d\n", tempColumna);
										printf("[+] Es el turno de %s.\n", arrPartidas[i].Jugadores[j - 1].nombre);
									}
								}
							} else if(strcmp("SALIR", cmd) == 0) {
				       			printf("[+] Partida %d interrumpida.\n", i);
						       	printf("[+] Jugador %s ha cerrado la conexión\n", arrPartidas[i].Jugador.nombre);
						       	fprintf(arrPartidas[i].Jugador.f, "SALIR\n");
								clearPartida(i);
							} else {
								salir_correctamente(EXIT_SUCCESS);
							}

							if (FD_ISSET(arrPartidas[i].Jugadores[j].fd, &except_fds)) {
					        	printf("except_fds for server.\n");
					        	salir_correctamente(EXIT_FAILURE);
					        }
						}
					}
				}

				/* Se comprueba si algún cliente nuevo desea conectarse y se le
				 * admite */
				if (FD_ISSET (server_socketfd, &readfds) ){

					/* Acepta la conexión con el cliente, guardándola en el array */
					addrlen = sizeof(client_dir);
					if ((client_socketfd = accept(server_socketfd, (struct sockaddr *)&client_dir, &addrlen)) != -1){
						printf("[+] Conexion establecida desde %s:%d\n", inet_ntoa(client_dir.sin_addr), ntohs(client_dir.sin_port));
					}else{
						perror("Failed Accept");
						exit(EXIT_FAILURE);
					}

					/* Nuevo jugador */
					jugador c;

					c.fd = client_socketfd;
					if( (c.f = fdopen(c.fd, "r+")) == NULL){
						perror("fdopen failed");
						protocolError(c.f, c.fd);
					}
					setbuf(c.f, NULL);
					
					if (numPartidas > MAX_PARTIDAS) {
						fprintf(c.f, "FULL\n");
						printf("[+] Nueva conexión denegada, número máximo de partidas alcanzado\n");
						protocolError(c.f, c.fd);
						goto H;
					} else {			

						/* Nueva partida */ 
						partida p;
						p.START_FLAG = FALSE;
						p.PLAYING_FLAG = FALSE;
						p.END_FLAG = FALSE;
						p.tablero = (int**)malloc(nFilas * sizeof(int*));
						for (int j = 0; j < nColumnas; ++j){
							p.tablero[j] = (int*)malloc(nColumnas * sizeof(int));
						}
						for (int i = 0; i < nFilas; ++i)
							for (int j = 0; j < nColumnas; ++j)
								p.tablero[i][j] = 0;

						arrPartidas[numPartidas] = p;
					}

					fprintf(c.f, "WELCOME\n");

					char cmd[MAXDATASIZE];
					c.user_id =  (char*)calloc(MAXUSERSIZE, sizeof(char));
					c.nombre = (char*)calloc(MAXNAMESIZE, sizeof(char));

					if(fgets(buffer, MAXDATASIZE, c.f) == 0){
						perror("fgets failed");
						protocolError(c.f, c.fd);
					}

					sscanf(buffer, "%s", cmd);
					if( strcmp("REGISTRAR", cmd) == 0){
						bzero(cmd, MAXDATASIZE);
						int a = random_number(0, 10);		/* Numeros de la operacion */	
						int b = random_number(3, 15);
						
						printf("[+] Recibida peticion de registro.\n");					
						printf("[+] Estableciendo prueba %d + %d. ", a , b);
						fflush(stdout);
						fprintf(c.f, "RESUELVE %d %d\n", a, b);
						if( fgets(buffer, MAXDATASIZE, c.f) == NULL){
							perror("1fgets failed");
							protocolError(c.f, c.fd);
						}
						
						sscanf(buffer, "%s", cmd);					
						if( strcmp("RESPUESTA", cmd) != 0){
							protocolError(c.f, c.fd);
						}
						sscanf(buffer, "%*s %d", &(c.res));

						/* 1.4 REGISTRADO */
						if( (c.res) == (a + b) ){	/* 1.4.1 REGISTRADO OK <id> */ 
							strcpy((c.user_id), randString());
							strcpy((c.nombre), "Invitado");
							printf("Recibido %d, prueba superada.\n", c.res);
							printf("[+] Asignando id %s.\n", c.user_id);	
							fprintf(c.f, "REGISTRADO OK %s\n", c.user_id);

							if(fgets(buffer, MAXDATASIZE, c.f) == NULL){
								perror("fgets failed");
								protocolError(c.f, c.fd);
							}
							sscanf(buffer, "%s", cmd);

							if(strcmp("SETNAME", cmd) == 0){
								sscanf(buffer, "%*s %s", c.nombre);
							}
							fprintf(c.f, "SETNAME OK\n");
							
							if((usersDB = fopen("users.db", "a+")) == NULL){
								perror("fopen failed");
								salir_correctamente(EXIT_FAILURE);
							}
							fprintf(usersDB, "%s %d %s\n", c.user_id, c.res, c.nombre);
							fclose(usersDB);
						}else{
							fprintf(c.f, "REGISTRADO ERROR\n");
							protocolError(c.f, c.fd);
						}
						
						c.player = 1;
						arrPartidas[numPartidas].Jugador = c;
						arrPartidas[numPartidas].START_FLAG = TRUE;
						printf("[+] La partida %d esta apunto de comenzar.\n", numPartidas);
						numPartidas++; 

					} else if( strcmp("LOGIN", cmd) == 0) {	
						
						char tempUser_id[MAXUSERSIZE];
						char tempName[MAXNAMESIZE];
						int tempRes;

						if ( (usersDB = fopen("users.db", "r")) == NULL) {
							perror("fopen failed");
							protocolError(c.f, c.fd);
						}			

						sscanf(buffer, "%*s %s %d", c.user_id, &c.res);
						bzero(buffer, MAXDATASIZE);

						/* Busqueda del usuario en la base de datos */
						while( fgets(buffer, MAXDATASIZE, usersDB) != NULL ){
							sscanf(buffer, "%s %d %s", tempUser_id, &tempRes, tempName);
							if( (strcmp(c.user_id, tempUser_id) == 0) && (c.res == tempRes) )
								break;						
						}

						/* Hemos encontrado el usuario */
						if ( (strcmp(c.user_id, tempUser_id) == 0) && (c.res == tempRes) ){
							strcpy((c.nombre), tempName);
							printf("[+] Usuario %s autentificado\n", c.user_id);
							fprintf(c.f, "LOGIN OK\n");
							fclose(usersDB);	
						} else {
							fprintf(c.f, "LOGIN ERROR\n");
							fclose(usersDB);
							protocolError(c.f, c.fd);
						}
						printf("[+] Numero de partidas en juego: %d\n", numPartidas);
						
						c.player = 1;
						arrPartidas[numPartidas].Jugador = c;
						arrPartidas[numPartidas].START_FLAG = TRUE;
						printf("[+] La partida %d esta apunto de comenzar.\n", numPartidas);
						numPartidas++; 

					}else{
						protocolError(c.f, c.fd);
					}	
				}
			}
		}
	return EXIT_SUCCESS;
}

/* Elimina las partidas finalizadas del array de partidas */
void compactaClavesP(void){

	if ((arrPartidas == NULL) || (numPartidas == 0) )
		return;

	int j = 0;
	for (int i = 0; i < numPartidas; i++){
		if (arrPartidas[i].END_FLAG != -1){
			arrPartidas[j] = arrPartidas[i];
			arrPartidas[i].numJugadores = 0;
			j++;
		}
	}
	
	numPartidas = j;
}

/* Mete una la ficha player en la columna col en el tablero, si es posible */
int meterFicha(int nCol, int nFil, char **matrix, int col, char player){
	
	if(col >= nCol)
		return -1;

	int z = 0; 
	while(z < nFil){
		if(matrix[z][col] != '-')
			break;
		z++;
	}
	if(z == 0){
		printf("[+] Columna incorrecta, vuelva a elegir.\n");
		return -1;
	}else{
		matrix[z - 1][col] = player;
		return 0;
	}
}


/* Comprueba si la partida a finlizado */
int connect4(int maxRow, int maxCol, char **tablero, char player)
{

    // Verificacion horizontal
    for (int j = 0; j < maxRow - 3 ; j++) {
        for (int i = 0; i < maxCol; i++) {
            if (tablero[i][j] == player && tablero[i][j+1] == player && tablero[i][j+2] == player && tablero[i][j+3] == player)
                return TRUE;       
        }
    }
    //Verificacion vertical
    for (int i = 0; i < maxCol - 3 ; i++ ) {
        for (int j = 0; j < maxRow; j++) {
            if (tablero[i][j] == player && tablero[i+1][j] == player && tablero[i+2][j] == player && tablero[i+3][j] == player)
                return TRUE;          
        }
    }
 
    // Verificacion diagonal ascendiente 
    for (int i = 3; i < maxCol; i++){
        for (int j = 0; j < maxRow - 3; j++) {
            if (tablero[i][j] == player && tablero[i-1][j+1] == player && tablero[i-2][j+2] == player && tablero[i-3][j+3] == player)
                return TRUE;
        }
    }

    // Verificacion diagonal descendiente
    for (int i = 3; i < maxCol; i++) {
        for (int j = 3; j < maxRow; j++) {
            if (tablero[i][j] == player && tablero[i-1][j-1] == player && tablero[i-2][j-2] == player && tablero[i-3][j-3] == player)
                return TRUE;
        }
    }

    return FALSE;
}

int max(int a, int b){
	return (a > b) ? a : b;
}

void clearPartida(int i){

	arrPartidas[i].END_FLAG = -1;
	//arrPartidas[i].numJugadores = 0;
	fclose(arrPartidas[i].Jugadores[0].f);
	fclose(arrPartidas[i].Jugadores[1].f);
	close(arrPartidas[i].Jugadores[0].fd);
	close(arrPartidas[i].Jugadores[1].fd);

}
