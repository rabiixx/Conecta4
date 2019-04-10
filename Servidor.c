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

/* Constantes */ 
#define MAXDATASIZE 2048	/* Tamaño del buffer */
#define MAXNAMESIZE 16
#define MAXUSERSIZE 12
#define MAX_CLIENTES 2
#define TRUE 1
#define FALSE 2

typedef struct _cliente{
	int fd;						/* Descriptor del socket cliente */
	FILE *f;					/* FILE del cliente para la comunicacion a mas alto nivel */
	char *nombre;				/* Nobre del cliente */
	char *user_id;				/* id del usuario */
	int res;					/* Resultado de la operacion, enviada por el cliente */
}cliente;

cliente *arrClientes;	 			/* Array con informacion de cada cliente en particular */
int numClientes = 0;
int server_socketfd;				/* Descriptor de fichero de socket servidor */

void salir_correctamente(int code);

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

void handle_signal_action(int sig_number)
{
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

	close(server_socketfd);
	  
	for (int i = 0; i < numClientes; ++i){
		fclose(arrClientes[i].f);
	    close(arrClientes[i].fd);
	}
  
	printf("Shutdown server properly.\n");
	exit(code);
}



void protocolError(FILE *f, int fd){

	printf("Socket Protocol Error.\n");
	numClientes--;
	fclose(f);
	close(fd);
}

/* Prototipos de las funciones definidas en este fichero */
//int dameMaximo (/*cliente *arrClientes, int numClientes*/);
void compactaClaves (/*cliente *arrClientes, int *numClientes*/);

int main(int argc, char const *argv[]){
	
	system("clear");
	
	if (argc != 4){
        perror("Usage: ./servidor <puerto> <numero_filas> <numero_columnas>");
        exit(EXIT_FAILURE);
    }

    if (setup_signals() != 0){
    	exit(EXIT_FAILURE);
    }


	/* Varables */ 
	int nFilas = atoi(argv[2]);
	int nColumnas = atoi(argv[3]);
	int tablero[nFilas][nColumnas];

	arrClientes = (cliente*)calloc(MAX_CLIENTES, sizeof(cliente));	
    int 				client_socketfd;			/* Descriptor de fichero de socket cliente */ 		
    struct sockaddr_in  server_dir;					/* Informacion sobre servidor */
    struct sockaddr_in  client_dir;					/* Informacion sobre direccion del cliente */
    socklen_t 			addrlen;					/* Tamaño de la informacion del cliente */
    char 				buffer[MAXDATASIZE];		/* Buffer General de 1024*sizeof(char) */
	int 				Puerto = atoi(argv[1]);		/* Puerto de escucha */    
	int running = 1;								/* Servidor en funcionamiento */
	FILE *usersDB = NULL;							/* Base de datos: Fichero con nombre de usuario, resultado y nombre de todos los usuarios */
	FILE *tempF = NULL;								/* Fichero utilizado para la modificacion de la base de datos(usersDB) */
	const char player1 = 'O';
	const char player2 = 'X';
	int TIE_FLAG = TRUE;
    /* Select */
	fd_set descriptoresLectura;						/* Descriptores de interes para select() */
	fd_set except_fds;								/* Control de excepciones */
	int maximo;										/* Número de descriptor más grande */						
	struct timeval selTimeout;						/* Timeout for select() */
	time_t server_start_t, end_t;					/* Tiempo de inicial, tiempo actual/final */
	time(&server_start_t);							/* Tiempo en el que el servidopr comienza su ejecucion */

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
	if ( listen(server_socketfd, MAX_CLIENTES) != -1){
		printf("[+] Escuchando conexiones en el puerto %d\n", Puerto);
	}else{
		printf("[+] Nueva conexión denegada, número máximo de clientes alcanzado\n");
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    /* Servidor activo */
	while (running){

		/********************/
		/* Comienza partida */
		/********************/

		/* START <name> <rows> <cols> */
		if(numClientes == 2){
			fprintf(arrClientes[0].f, "START %s %d %d", arrClientes[1].nombre, nFilas, nColumnas);
			fprintf(arrClientes[0].f, "START %s %d %d", arrClientes[0].nombre, nFilas, nColumnas);
			
			/* Quien empieza jugando ? */
			fprintf(arrClientes[0].f, "URTURN\n");
			for (int i = 0; i < 6; ++i)
				for (int j = 0; j < 6; ++j)
					matrix[i][j] = 'A';

			while(1){

				for (int i = 0; i < 6; ++i)
		            for (int j = 0; j < 6; ++j){
		                if(tablero[i][j] == -1){
		                    TIE_FLAG = FALSE;
		                    break;
		                }
		            }

				int gameState = partidaFin(nColumnas, nFilas, tablero);
				if(checkWin(nFilas, nColumnas, tablero, player1) == TRUE){
					fprintf(arrClientes[0].f, "VICTORY\n");
					fprintf(arrClientes[1].f, "DEFEAT\n");
					break;
				}else if(checkWin(nFilas, nColumnas, tablero, player2) == TRUE){
					fprintf(arrClientes[0].f, "DEFEAT\n");
					fprintf(arrClientes[1].f, "VICTORY\n");
					break
				}else if(TIE_FLAG == TRUE){
					fprintf(arrClientes[0].f, "TIE\n");
					fprintf(arrClientes[1].f, "TIE\n");
				}else{

					compactaClaves();
				
					FD_ZERO (&readfds);
					FD_SET(server_socketfd, &readfds);
					maximo = server_socketfd;

					/* Se añaden para select() los sockets con los clientes ya conectados */
					for (int i = 0; i < numClientes; i++){
						FD_SET (arrClientes[i].fd, &descriptoresLectura);
						maximo = (arrClientes[i].fd > maximo) ? arrClientes[i].fd : maximo;
					}

					FD_ZERO (&except_fds);
					FD_SET(server_socketfd, &except_fds);
					for (int i = 0; i < numClientes; ++i)
						FD_SET(arrClientes[i].fd, &except_fds);
				
					int activity = select (maximo + 1, &descriptoresLectura, NULL, &except_fds, NULL);
				    
				    switch (activity) {
				      	case -1:
				        	perror("select()");
				        	salir_correctamente(EXIT_FAILURE);

				      	case 0:
				       		/*for (int i = 0; i < numClientes; ++i){
								time(&end_t);
								printf("[+] Enviando información de tiempo a %s\n", arrClientes[i].user_id);
								fprintf(arrClientes[i].f, "UPTIME %f %f\n", difftime(end_t, server_start_t), difftime(end_t, arrClientes[i].client_start_t));
							}*/
				      default:

						for (int i = 0; i < numClientes; i++){
							if(FD_ISSET(arrClientes[i].fd, &descriptoresLectura) ){

								int tempColumna;
								if(fgets(buffer, MAXDATASIZE, arrClientes[i].f) == NULL){
									perror("fgets failed");
									salir_correctamente(EXIT_FAILURE);
								}

								sscanf(buffer, "%s", cmd);
								if(strcmp("COLUMN", cmd) == 0){
									sscanf(buffer, "%s")
									if (meterFicha(nColumnas, nColumnas, tablero, int tempColumna) == -1){
										fprintf(arrClientes[i].f "COLUMN ERROR\n");
									}else{
										fprintf(arrClientes[i].f "COLUMN OK\n");
									}
								}
								
								



							}
												
							if (FD_ISSET(arrClientes[i].fd, &except_fds)) {
					        	printf("except_fds for server.\n");
					        	salir_correctamente(EXIT_FAILURE);
					        }
						}
					}
				}
				}





					


		
			
		/* Se comprueba si algún cliente nuevo desea conectarse y se le
		 * admite */
		if (FD_ISSET (server_socketfd, &descriptoresLectura) ){

			/* Nuevo cliente */
			cliente c;
			(numClientes)++;

			/* Acepta la conexión con el cliente, guardándola en el array */
			addrlen = sizeof(client_dir);
			if ((client_socketfd = accept(server_socketfd, (struct sockaddr *)&client_dir, &addrlen)) != -1){
				printf("[+] Conexion establecida desde %s:%d\n", inet_ntoa(client_dir.sin_addr), ntohs(client_dir.sin_port));
			}else{
				perror("Failed Accept");
				exit(EXIT_FAILURE);
			}

			c.fd = client_socketfd;
			if( (c.f = fdopen(c.fd, "r+")) == NULL){
				perror("fdopen failed");
				protocolError(c.f, c.fd);
			}
			setbuf(c.f, NULL);

			/* Si se ha superado el maximo de clientes, se cierra la conexión,
			 * se deja todo como estaba y se vuelve. */
			if ((numClientes) > MAX_CLIENTES){
				fprintf(c.f, "FULL\n");
				printf("[+] Nueva conexión denegada, número máximo de clientes alcanzado\n");
				protocolError(c.f, c.fd);
			}else{

				fprintf(c.f, "WELCOME\n");

				char cmd[MAXDATASIZE];
				c.user_id =  (char*)calloc(MAXUSERSIZE, sizeof(char));
				c.nombre = (char*)calloc(MAXNAMESIZE, sizeof(char));
				time(&(c.client_start_t));

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
						perror("fgets failed");
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

					if(fgets(buffer, MAXDATASIZE, c.f) == NULL){
						perro("fgets failed");
						protocolError(c.f, c.fd);
					}
					sscanf(buffer, "%s", cmd);

					if(strcmp("SETNAME", cmd) == 0){
						sscanf(buffer, "%*s %s", c.nombre);
					}
	
					arrClientes[numClientes - 1] = c;
				}else if( strcmp("LOGIN", cmd) == 0){	
					char tempUser_id[MAXUSERSIZE];
					char tempName[MAXNAMESIZE];
					int tempRes;

					if((usersDB = fopen("users.db", "r")) == NULL){
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
					if( (strcmp(c.user_id, tempUser_id) == 0) && (c.res == tempRes) ){
						strcpy((c.nombre), tempName);
						printf("[+] Usuario %s autentificado\n", c.user_id);
						fprintf(c.f, "LOGIN OK\n");
						fclose(usersDB);	
					}else{
						fprintf(c.f, "LOGIN ERROR\n");
						fclose(usersDB);
						protocolError(c.f, c.fd);
					}
					printf("[+] Numero de usuarios conectados: %d\n",numClientes );
					arrClientes[numClientes - 1] = c;
				}else{
					protocolError(c.f, c.fd);
				}

			}
		}
	}
}

	return EXIT_SUCCESS;
}

void compactaClaves(/*cliente *arrClientes, int *numClientes*/void){

	if ((arrClientes == NULL) || ((numClientes) == 0))
		return;

	int j = 0;
	for (int i = 0; i < (numClientes); i++){
		if (arrClientes[i].fd != -1){
			arrClientes[j] = arrClientes[i];
			j++;
		}
	}
	
	numClientes = j;
}

/*https://gist.github.com/Alexey-N-Chernyshov/4634731*/


void meterFicha(int nCol, int nFil, char matrix[][nCol], int col){
	
	int i = 0; 
	while(i < nFil){
		if(matrix[i][col] != 'A')
			break;
		z++;
	}
	if(z == 0){
		return -1;
	}else{
		matrix[z - 1][col] = 'O';
		return 0;
	}
}


int connect4(int maxRow, int maxCol, int tablero[][maxCol], int player){

    // horizontalCheck 
    for (int j = 0; j<maxRow-3 ; j++ ){
        for (int i = 0; i<maxCol; i++){
            if (tablero[i][j] == player && tablero[i][j+1] == player && tablero[i][j+2] == player && tablero[i][j+3] == player){
                return TRUE;
            }           
        }
    }
    // verticalCheck
    for (int i = 0; i<maxCol-3 ; i++ ){
        for (int j = 0; j<maxRow; j++){
            if (tablero[i][j] == player && tablero[i+1][j] == player && tablero[i+2][j] == player && tablero[i+3][j] == player){
                return TRUE;
            }           
        }
    }
    // ascendingDiagonalCheck 
    for (int i=3; i<maxCol; i++){
        for (int j=0; j<maxRow-3; j++){
            if (tablero[i][j] == player && tablero[i-1][j+1] == player && tablero[i-2][j+2] == player && tablero[i-3][j+3] == player)
                return TRUE;
        }
    }
    // descendingDiagonalCheck
    for (int i=3; i<maxCol; i++){
        for (int j=3; j<maxRow; j++){
            if (tablero[i][j] == player && tablero[i-1][j-1] == player && tablero[i-2][j-2] == player && tablero[i-3][j-3] == player)
                return TRUE;
        }
    }

    return FALSE;



}

int main(int argc, char const *argv[]){

    int maxRow = 6;
    int maxCol = 6;
    int tablero[6][6] = {
                                {0, 0, 0, 0, 0, 0}, 
                                {0, 1, 0, 0, 0, 0},
                                {1, 0, 0, 0, 1, 0},
                                {0, 0, 0, 0, 0, 0},
                                {0, 0, 1, 0, 0, 0},
                                {0, 0, 0, 1, 0, 0}
                                };

    const int player1 = 1;
    const int player2 = 2;

    if(connect4(maxRow, maxCol, tablero, player1) == TRUE){
        printf("Victory!\n");
    }else{
        printf("Defeat!\n");
    }
}
