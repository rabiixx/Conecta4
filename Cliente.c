/* 
* Nombre del programa: cliente1.c
* Autores: Ruben Cherif y Jonnhy Chicaiza
* Descripcion: Objetivo 3 - Cambio de nombre y obtencion del mismo
* Fecha: 10/02/2019 - 14:32
* Version: 4.0
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

/* Informacion sobre el usuario */
typedef struct _login{
	char user_id[MAXUSERSIZE];	/* id del usuario */
	int res;			/* Resultado de la operacion, enviada por el cliente */
}log;

int socket_fd;	
FILE *server_f = NULL;										/* Fichero utilizado para la cominicacion de sockets */ 

void salir_correctamente(int code);
int meterFicha(int nCol, int nFil, char matrix[][nCol], int col, char player);

void handle_signal_action(int sig_number)
{
  	if (sig_number == SIGINT) {
    	printf("SIGINT was catched!\n");
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

  	return 0;
}

void salir_correctamente(int code){

	fprintf(server_f, "SALIR\n");
	fclose(server_f);
	close(socket_fd);
	printf("\n[+] Shutdown client properly.\n");
	exit(code);
}

void protocolError(int code){

	fclose(server_f);
	close(socket_fd);
	printf("[+] Shutdown client properly.\n");
	exit(code);
}

/* Programa Principal */
int main(int argc, char *argv[]){

	system("clear");

	if(argc != 3){
		perror("Usage: ./client <ip_address> <port> ");
		exit(EXIT_FAILURE);
	}

 	if (setup_signals() != 0)
    	exit(EXIT_FAILURE);

	/* Variables. Importante inicializar */ 
	struct sockaddr_in server;
	char *IP = argv[1];											/* Direccion IP */
	int PORT = atoi(argv[2]);									/* Numero del puerto */	
	char *buffer = (char*)calloc(MAXDATASIZE, sizeof(char));	/* Data buffer */
	log Login;													/* Informacion sobre el usuario */
	FILE *user_f = NULL;										/* Fichero que contiene el id del usuario y el resualtado de la adicion: <id> <c> */

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

	if( access("username", F_OK) == -1){
		
		int a, b;
		char cmd[MAXDATASIZE];

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
		sscanf(buffer,"%*s %d %d", &a, &b);
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
				fclose(user_f);
			}else
				protocolError(EXIT_FAILURE);
		}else
			protocolError(EXIT_FAILURE);
	
	}else if(access("username", F_OK) != -1){	
		
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

	while(1){

		int nFilas, nColumnas, opponent;
		char nombreVS[16];
		char player = 'O';
		char playerVS = 'X';

		if(fgets(buffer, MAXDATASIZE, server_f) ==  NULL){
			perror("fgets failed");
			salir_correctamente(EXIT_FAILURE);
		}
		sscanf(buffer, "%s", cmd);
		if(strcmp("START", cmd ) != 0){
			salir_correctamente(EXIT_FAILURE);
		}
		bzero(cmd, MAXDATASIZE);
		sscanf("%*s %s %d %d", nombreVS, &nFilas, &nColumnas);
		char tablero[nFilas][nColumnas];
		for (int i = 0; i < count; ++i)
			for (int i = 0; i < count; ++i)
				tablero[i][j] = 'A';
		printf("[+] Nombre adversario: %s\n", nombreVS);

		if(fgets(buffer, MAXDATASIZE, server_f) ==  NULL){
			perror("fgets failed");
			salir_correctamente(EXIT_FAILURE);
		}
		sscanf(buffer,"%s", cmd);
		if(strcmp("URTURN", cmd) != 0){
			salir_correctamente(EXIT_FAILURE);
		}else if(strcmp("VICTORY", cmd) == 0){
			printf("[+] VICTORIA!\n");
			salir_correctamente(EXIT_SUCCESS);
		}else if(strcmp("DEFEAT", cmd) == 0){
			printf("[+] DERROTA!\n");
			salir_correctamente(EXIT_SUCCESS);
		}else if(strcmp("TIE", cmd) == 0){
			printf("[+] TIE\n");
			salir_correctamente(EXIT_SUCCESS);
		}

		sscanf(buffer,"%*s %d", &opponent);
		meterFicha(nFilas, nColumnas, tablero, opponent, player);
        for (int i = 0; i < nFilas; ++i){
	    	for (int j = 0; j < nColumnas; ++j){
	    		printf("| %c |\t", tablero[i][j]);
	    	}
	    	printf("\n");
	    }	

C:		printf("[+] Introduce la columna donde desee meter la ficha: ");
		scanf("%d", col);
		fprintf(server_f, "COLUMN %s\n", col);

		if(fgets(buffer, MAXDATASIZE, server_f) ==  NULL){
			perror("fgets failed");
			salir_correctamente(EXIT_FAILURE);
		}
		if(strcmp("COLUMN OK", buffer) == 0){
			meterFicha(nFilas, nColumnas, tablero, opponent, player);
		}else if(strcmp("COLUMN ERROR", buffer) == 0)
			goto C;
		else{
			salir_correctamente(EXIT_FAILURE);
		}
	
	}
}


int meterFicha(int nCol, int nFil, char matrix[][nCol], int col, char player){
	
	int z = 0; 
	while(z < nFil){
		if(matrix[z][col] != 'A')
			break;
		z++;
	}
	if(z == 0){
		return -1;
	}else{
		matrix[z - 1][col] = player;
		return 0;
	}
}