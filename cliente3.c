/* 
* Nombre del programa: cliente3.c
* Autores: Ruben Cherif y Jonnhy Chicaiza
* Descripcion: Conecta4 - Objetivo 3
* Version: 1.0
*/

/* Librerias */
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <limits.h>

#define MAXDATASIZE 2048	/* Tamaño del buffer */
#define MAXUSERSIZE 12		/* Tamaño id del usuario */
#define MAXNAMESIZE 16
#define TRUE 1
#define FALSE 0


/* Informacion sobre el usuario */
typedef struct _login{
	char user_id[MAXUSERSIZE];		/* id del usuario */
	int res;						/* Resultado de la operacion, enviada por el cliente */
}log;

/* Variables globales */
int socket_fd;						
FILE *server_f = NULL;				/* Fichero utilizado para la cominicacion de sockets */ 

/* Declaracion de funciones */
void salir_correctamente(int code);

void meterFicha(int nCol, int nFil, char tablero[][nCol], int col, char player);

void login();

void registrar(char nombre[MAXDATASIZE]);

int finPartida(char cmd[MAXDATASIZE]);

void mostrarTablero(int nFil, int nCol, char tablero[][nCol]);

/* Definicion de funciones */

/* Captura la señal de CTRL+C */
void handle_signal_action(int sig_number)
{
  	if (sig_number == SIGINT) {
    	printf("SIGINT was catched!\n");
    	salir_correctamente(EXIT_SUCCESS);
  	}

}

/* Handler de sañal CTRL+C */
int setup_signals()
{
  	struct sigaction sa;
  	sa.sa_handler = handle_signal_action;
  	if (sigaction(SIGINT, &sa, 0) != 0) {
    	perror("sigaction()");
    	return -1;
  	}

  	return 0;
}

/* Envia SALIR\n al servidor, cierra la conexion con el servidor y sale */
void salir_correctamente(int code)
{
	fprintf(server_f, "SALIR\n");
	fclose(server_f);
	close(socket_fd);
	printf("\n[+] Shutdown client properly.\n");
	exit(code);
}

/* Error de protocolo por parte del servidor */
void protocolError(int code){

	fclose(server_f);
	close(socket_fd);
	printf("[+] Shutdown client properly.\n");
	exit(code);
}

/* Programa Principal */
int main(int argc, char *argv[]){

	system("clear");

	if(argc != 4){
		perror("Usage: ./client <ip_address> <port> <nombre>");
		exit(EXIT_FAILURE);
	}

 	if (setup_signals() != 0)
    	exit(EXIT_FAILURE);

	/* Variables. Importante inicializar */ 
	struct sockaddr_in server;
	char *IP = argv[1];											/* Direccion IP */
	int PORT = atoi(argv[2]);									/* Numero del puerto */	
	char *buffer = (char*)calloc(MAXDATASIZE, sizeof(char));	/* Data buffer */

	/* Create socket */
	if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Failed to create socket");
		exit(EXIT_FAILURE);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);					
	server.sin_addr.s_addr = inet_addr(IP);		
	memset(&(server.sin_zero), '\0', 8);	/* Ponemos a cero el resto de la estructura */
	
	/* Connect */
	if( connect(socket_fd, (struct sockaddr *)&server, sizeof(server)) == -1){
		perror("Connect failed");
		exit(EXIT_FAILURE);
	}

	if( (server_f = fdopen(socket_fd, "w+")) == NULL){
		perror("fdopen failed");
		exit(EXIT_FAILURE);
	}
	setbuf(server_f, NULL);
	
	/*************/
	/* PROTOCOLO */
	/*************/

	if( fgets(buffer, MAXDATASIZE, server_f) == NULL){
		perror("fgets failed");
		protocolError(EXIT_FAILURE);
	}

	if(strcmp("FULL\n", buffer) == 0){
		salir_correctamente(EXIT_SUCCESS);
	}else if(strcmp("WELCOME\n", buffer) != 0){
		salir_correctamente(EXIT_FAILURE);
	}

	if ( access("username", F_OK) == -1)
		registrar(argv[3]);
	else if (access("username", F_OK) != -1)	
		login();

	/* Comienzo del juego */
	char cmd[MAXDATASIZE];
	int nFilas, nColumnas;			/* Numero de filas y columnas de la matriz */
	char nombreVS[MAXNAMESIZE];		/* Nombre del jugador contrincante */
	char player = 'O';				/* Mi ficha */
	char playerVS = 'X';			/* Ficha del jugador contrincante */
	
	/* START */
	if (fgets(buffer, MAXDATASIZE, server_f) ==  NULL){
		perror("fgets failed");
		salir_correctamente(EXIT_FAILURE);
	}
	sscanf(buffer, "%s", cmd);

	if(strcmp("START", cmd ) != 0){
		salir_correctamente(EXIT_FAILURE);
	}
	bzero(cmd, MAXDATASIZE);

	sscanf(buffer, "%*s %s %d %d", nombreVS, &nFilas, &nColumnas);
	char tablero[nFilas][nColumnas];
	for (int i = 0; i < nFilas; ++i)
		for (int j = 0; j < nColumnas; ++j)
			tablero[i][j] = '-';
	
	printf("[+] Nombre del adversario: %s\n", nombreVS);

	while(1){

		int opponent;		/* Columna en la que el jugador adversario metio la ficha por ultima vez */			
		int col;			/* Almacenar donde meto la ficha */
		
		printf("[+] Esperando a que el adversario responda...\n");
		if (fgets(buffer, MAXDATASIZE, server_f) ==  NULL) {
			perror("fgets failed");
			salir_correctamente(EXIT_FAILURE);
		}
		sscanf(buffer,"%s", cmd);

		if (finPartida(cmd) == TRUE)
			salir_correctamente(EXIT_SUCCESS);

		if (strcmp("URTURN\n", buffer) == 0) {
			printf("[+] Empiezas jugando: \n");
			mostrarTablero(nFilas, nColumnas, tablero);

	B:		printf("[+] Introduce la columna donde desee meter la ficha: ");
			scanf("%d", &col);
			fprintf(server_f, "COLUMN %d\n", col);

			if (fgets(buffer, MAXDATASIZE, server_f) ==  NULL){
				perror("fgets failed");
				salir_correctamente(EXIT_FAILURE);
			}

			if (strcmp("COLUMN OK\n", buffer) == 0)
				meterFicha(nFilas, nColumnas, tablero, col, player);
			else if (strcmp("COLUMN ERROR\n", buffer) == 0)
				goto B;
			else
				salir_correctamente(EXIT_FAILURE);

		} else {
			sscanf(buffer,"%*s %d", &opponent);
			meterFicha(nFilas, nColumnas, tablero, opponent, playerVS);
	       	mostrarTablero(nFilas, nColumnas, tablero);

	C:		printf("[+] Introduce la columna donde desee meter la ficha: ");
			scanf("%d", &col);
			fprintf(server_f, "COLUMN %d\n", col);

			if (fgets(buffer, MAXDATASIZE, server_f) ==  NULL){
				perror("fgets failed");
				salir_correctamente(EXIT_FAILURE);
			}

			if (strcmp("COLUMN OK\n", buffer) == 0)
				meterFicha(nFilas, nColumnas, tablero, col, player);
			else if (strcmp("COLUMN ERROR\n", buffer) == 0)
				goto C;
			else
				salir_correctamente(EXIT_FAILURE);
		}
	}
}

/* Se mete la ficha player en la columna col del tablero */
void meterFicha(int nCol, int nFil, char tablero[][nCol], int col, char player)
{
	int i;
	for (i = 0; i < nFil; ++i) 
		if (tablero[i][col] != '-')
			break;		

	tablero[i - 1][col] = player;
}

void login()
{

	FILE *user_f = NULL;		/* Fichero que contiene el id del usuario y el resualtado de la adicion: <id> <c> */
	log Login;													/* Informacion sobre el usuario */
	char buffer[MAXDATASIZE];

	if( (user_f = fopen("username", "r")) == NULL){
		perror("fopen failed");
		protocolError(EXIT_FAILURE);
	}
	
	fscanf(user_f, "%s %d", Login.user_id, &Login.res);
	printf("[+] Hay datos para el usuario %s, probamos autentificación\n", Login.user_id);
	fclose(user_f);
	
	fprintf(server_f, "LOGIN %s %d\n", Login.user_id, Login.res);
	bzero(buffer, MAXDATASIZE);

	if( fgets(buffer, MAXDATASIZE, server_f) == NULL){
		perror("fgets failed");
		exit(EXIT_FAILURE);
	}

	if(strcmp("LOGIN OK\n", buffer) == 0){
		printf("[+] Sesion establecida con id %s\n", Login.user_id);
	}else if (strcmp("LOGIN ERROR\n", buffer) == 0){
		printf("[+] Error al registrar\n");
		protocolError(EXIT_FAILURE);
	}else
		protocolError(EXIT_FAILURE);
}

void registrar(char nombre[MAXNAMESIZE])
{
	int a, b;
	char buffer[MAXDATASIZE];
	char cmd[MAXDATASIZE];
	FILE *user_f = NULL;		/* Fichero que contiene el id del usuario y el resualtado de la adicion: <id> <c> */
	log Login;


	printf("[+] No existe id almacenado, realizando petición de registro\n");
	fprintf(server_f, "REGISTRAR\n");

	if( fgets(buffer, MAXDATASIZE, server_f) == NULL){
		perror("fgets failed");
		protocolError(EXIT_FAILURE);
	}

	sscanf(buffer, "%s", cmd);
	if(strcmp("RESUELVE", cmd) != 0){
		protocolError(EXIT_FAILURE);
	}
	bzero(cmd, MAXDATASIZE);
	sscanf(buffer, "%*s %d %d", &a, &b);
	printf("[+] Resuelve %d + %d = ", a, b);

	bzero(buffer, MAXDATASIZE);
	Login.res = a + b;
	if( (Login.res < 0) || (Login.res > INT_MAX) || (Login.res > INT_MAX))
		protocolError(EXIT_FAILURE);
	printf("%d\n", Login.res);
		

	fprintf(server_f, "RESPUESTA %d\n", Login.res);
	if( fgets(buffer, MAXDATASIZE, server_f) == NULL){
		perror("fgets failed");
		protocolError(EXIT_FAILURE);
	}

	sscanf(buffer, "%s", cmd);
	if(strcmp("REGISTRADO ERROR\n", buffer) == 0){
		printf("[+] Error al registrar\n");
		protocolError(EXIT_FAILURE);		
	}else if(strcmp("REGISTRADO", cmd) == 0){
		bzero(cmd, MAXDATASIZE);
		sscanf(buffer, "%*s %s", cmd);
		if(strcmp("OK", cmd) == 0){
			sscanf(buffer, "%*s %*s %s", Login.user_id);
			printf("[+] Sesión establecida con id %s\n", Login.user_id);			
			printf("[+] Conexion abierta\n");
			
			if( (user_f = fopen("username", "w+")) == NULL){	
				perror("fopen failed");
				exit(EXIT_FAILURE);	
			}

			fprintf(user_f, "%s %d\n", Login.user_id, Login.res);
			fprintf(server_f, "SETNAME %s\n", nombre);

			if( fgets(buffer, MAXDATASIZE, server_f) == NULL){
				perror("fgets failed");
				exit(EXIT_FAILURE);
			}

			if(strcmp("SETNAME OK\n", buffer) != 0){
				salir_correctamente(EXIT_SUCCESS);
			}

			printf("[+] Nombre establecido correctamente.\n");

			fclose(user_f);
		}else
			protocolError(EXIT_FAILURE);
	}else
		protocolError(EXIT_FAILURE);
}

/* Comprueba si la partida ha finalizado */
int finPartida(char cmd[MAXDATASIZE])
{

	if (strcmp("VICTORY", cmd) == 0) {
		printf("[+] VICTORIA!\n");
		return TRUE;
	} else if (strcmp("DEFEAT", cmd) == 0) {
		printf("[+] DERROTA!\n");
		return TRUE;
	} else if(strcmp("TIE", cmd) == 0) {
		printf("[+] TIE\n");
		return TRUE;
	} else if(strcmp("SALIR", cmd) == 0){
		printf("[+] El jugador contrincante ha abandonado la partida.\n");
		return TRUE;
	} else if(strcmp("URTURN", cmd) != 0) {
		return TRUE;
	}

	return FALSE;

}

/* Muestra el estado del tablero por pantalla */
void mostrarTablero(int nFil, int nCol, char tablero[][nCol])
{
	for (int i = 0; i < nFil; ++i){
    	for (int j = 0; j < nCol; ++j){
    		printf("| %c |\t", tablero[i][j]);
    	}
    	printf("\n");
    }
}