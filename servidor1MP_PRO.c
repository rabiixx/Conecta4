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

/* Constantes */ 
#define MAXDATASIZE 2048	/* Tamano del buffer */
#define MAXNAMESIZE 16		/* Maximo tamano del nombre */
#define MAXUSERSIZE 12		/* Maximo tamano del nombre de usuario */
#define MAX_JUGADORESP 2	/* Maximo numero de jugadores por partida */
#define MAX_PARTIDAS 4		/* Numero maximo de partidas */
#define TRUE 1
#define FALSE 0


/* Informacion sobre jugador */
typedef struct _jugador{
	int fd;						/* Descriptor del socket cliente */
	FILE *f;					/* FILE del cliente para la comunicacion a mas alto nivel */
	char nombre[MAXNAMESIZE + 1];				/* Nobre del cliente */
	char user_id[MAXUSERSIZE + 1];				/* id del usuario */
	int res;					/* Resultado de la operacion, enviada por el cliente */
	int player;					/* Ficha del jugador */
}jugador;

/* Informacion sobre la partida */
typedef struct _partida{
	jugador Jugadores[MAX_JUGADORESP];		/* Array de los jugadores que jugaran la partidas */
	int **tablero;							/* Tablero de la partida */
	int START_FLAG;							/* Indica que la partida esta lista para comenzar */
	int PLAYING_FLAG;						/* Indica que la partida esta en juego */
	int END_FLAG;							/* Indica que la partidas ha finlizado */
	int TIE_FLAG;
	int numJugadores;						/* Indica el numero de jugadores conectados a la partida */
}partida;

/* Variables globales */
int numClientes = 0;				/* Numero de clientes conectados */
int numPartidas = 0;				/* Numero de partidas que se estan jugandose */
int server_socketfd;				/* Descriptor de fichero de socket servidor */
partida arrPartidas[MAX_PARTIDAS];	/* Array con los datos de cada partida */
FILE *usersDB = NULL;							/* Base de datos: Fichero con nombre de usuario, resultado y nombre de todos los usuarios */


/* Definicion de funciones */
void login(jugador j, char buffer[MAXDATASIZE]);

void nuevoJugador(int nFil, int nCol);

void registrar(jugador c);

void comienzaPartida(int i, int nFil, int nCol);

void mostrarTablero(int nFil, int nCol, int **tablero);

void turno(int p_index, int j_index, int tempColumna);

int connect4(int maxRow, int maxCol, int **tablero, int player, int p);

int meterFicha(int nFil, int nCol, int **matrix, int col, int player);

void finPartida(int p_index, int j_index);

void salir(int i, int j);

void clearPartida(int);

void compactaClavesP(void);

void salir_correctamente(int code);

/* Declaracion de funciones auxiliares*/
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

void salir_correctamente(int code)
{
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

void protocolError(FILE *f, int fd)
{
	fclose(f);
	close(fd);
}

int max(int a, int b)
{
	return (a > b) ? a : b;
}


int main(int argc, char const *argv[])
{
	
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
    struct sockaddr_in  server_dir;					/* Informacion sobre servidor */
    char 				buffer[MAXDATASIZE];		/* Buffer General de 1024*sizeof(char) */
	int 				Puerto = atoi(argv[1]);		/* Puerto de escucha */    
	int running = 1;								/* Servidor en funcionamiento */
    
    for (int i = 0; i < MAX_PARTIDAS; ++i){
    	arrPartidas[i].numJugadores = 0;
    }


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
H:		for (int i = 0; i < numPartidas; ++i) {
			if (arrPartidas[i].START_FLAG == TRUE)
				comienzaPartida(i, nFilas, nColumnas);
		}

		compactaClavesP();

		printf("[+] Numero de partidas en juego: %d\n", numPartidas);
		for (int i = 0; i < numPartidas; ++i) {
			printf("\t- Partida %d: %s VS %s\n", i, arrPartidas[i].Jugadores[0].nombre, arrPartidas[i].Jugadores[1].nombre);
		}
	
		FD_ZERO (&readfds);
		FD_SET (server_socketfd, &readfds);
		FD_ZERO (&except_fds);
		FD_SET (server_socketfd, &except_fds);

		for (int i = 0; i < numPartidas; ++i){
			FD_SET(arrPartidas[i].Jugadores[0].fd, &readfds);
			FD_SET(arrPartidas[i].Jugadores[1].fd, &readfds);
			FD_SET(arrPartidas[i].Jugadores[0].fd, &except_fds);
			FD_SET(arrPartidas[i].Jugadores[1].fd, &except_fds);
			maximo = max(arrPartidas[i].Jugadores[0].fd, arrPartidas[i].Jugadores[1].fd);
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
A:							if (fgets(buffer, MAXDATASIZE, arrPartidas[i].Jugadores[j].f) == NULL){
								perror("fgets failed");
								salir_correctamente(EXIT_FAILURE);
							}

							sscanf(buffer, "%s", cmd);
							if (strcmp("COLUMN", cmd) == 0) {
								sscanf(buffer, "%*s %d", &tempColumna);
								if (meterFicha(nColumnas, nColumnas,  arrPartidas[i].tablero, tempColumna, arrPartidas[i].Jugadores[j].player) == -1) {
									fprintf(arrPartidas[i].Jugadores[j].f, "COLUMN ERROR\n");
									goto A;
								} else {

									fprintf(arrPartidas[i].Jugadores[j].f, "COLUMN OK\n");	

									printf("[+] Partida %d: %s VS %s\n", i, arrPartidas[i].Jugadores[0].nombre, arrPartidas[i].Jugadores[1].nombre);
									mostrarTablero(nFilas, nColumnas, arrPartidas[i].tablero);

								   	if (connect4(nFilas, nColumnas, arrPartidas[i].tablero, arrPartidas[i].Jugadores[j].player, i) == TRUE) {
								   		finPartida(i, j);
								    } else {
								    	turno(i, j, tempColumna);
								   	}
								}
							} else if(strcmp("SALIR", cmd) == 0) {
								salir(i, j);
							} else {
								salir_correctamente(EXIT_SUCCESS);
							}
						}

						if (FD_ISSET(arrPartidas[i].Jugadores[j].fd, &except_fds)) {
				        	printf("except_fds for server.\n");
				        	salir_correctamente(EXIT_FAILURE);
				        }
					}
				}

				/* Se comprueba si algún cliente nuevo desea conectarse y se le
				 * admite */
				if (FD_ISSET (server_socketfd, &readfds) ){
					nuevoJugador(nFilas, nColumnas);
				}
			}
		}
	return EXIT_SUCCESS;
}

/* Elimina las partidas finalizadas del array de partidas */
void compactaClavesP(void)
{
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

/* Introduce la ficha player en la columna col del tablero. 
 * Si la ficha no se puede meter devuelve FALSE, si no TRUE */
int meterFicha(int nFil, int nCol, char **matrix, int col, char player)
{
	
	if(col >= nCol)
		return FALSE;

	int z = 0; 
	while (z < nFil) {
		if(matrix[z][col] != '-')
			break;
		z++;
	}
	if (z == 0) { 
		printf("[+] Columna incorrecta, vuelva a elegir.\n");
		return FALSE;
	} else {
		matrix[z - 1][col] = player;
		return TRUE;
	}
}


/* Comprueba si la partida a finlizado */
int connect4(int nFil, int nCol, int **tablero, int player, int p)
{

    // Verificacion horizontal
    for (int j = 0; j < nFil - 3 ; j++ ){
        for (int i = 0; i < nCol; i++){
            if (tablero[i][j] == player && tablero[i][j+1] == player && tablero[i][j+2] == player && tablero[i][j+3] == player){
                return TRUE;
            }           
        }
    }
    //Verificacion vertical
    for (int i = 0; i < nCol - 3 ; i++ ){
        for (int j = 0; j < nFil; j++){
            if (tablero[i][j] == player && tablero[i+1][j] == player && tablero[i+2][j] == player && tablero[i+3][j] == player){
                return TRUE;
            }           
        }
    }
 
    // Verificacion diagonal ascendiente 
    for (int i = 3; i < nCol; i++){
        for (int j=0; j < nFil - 3; j++){
            if (tablero[i][j] == player && tablero[i-1][j+1] == player && tablero[i-2][j+2] == player && tablero[i-3][j+3] == player)
                return TRUE;
        }
    }

    // Verificacion diagonal descendiente
    for (int i = 3; i < nCol; i++) {
        for (int j = 3; j < nFil; j++) {
            if (tablero[i][j] == player && tablero[i-1][j-1] == player && tablero[i-2][j-2] == player && tablero[i-3][j-3] == player)
                return TRUE;
        }
    }

    /* Verificacion empate */
	for (int i = 0; i < nFil; ++i)
        for (int j = 0; j < nCol; ++j){
            if (tablero[i][j] == 0)
                goto E;
            else if( (i == (nFil - 1)) && (j == (nCol - 1)) ) 
        		arrPartidas[p].TIE_FLAG = TRUE;
        }

E:    return (arrPartidas[p].TIE_FLAG == TRUE) ? TRUE : FALSE; 

}

void clearPartida(int i)
{
	arrPartidas[i].END_FLAG = -1;
	fclose(arrPartidas[i].Jugadores[0].f);
	fclose(arrPartidas[i].Jugadores[1].f);
	close(arrPartidas[i].Jugadores[0].fd);
	close(arrPartidas[i].Jugadores[1].fd);
}

/* Acepta la conexión con el cliente, guardándola en el array */
void nuevoJugador(int nFilas, int nColumnas){

	char buffer[MAXDATASIZE];
	socklen_t 			addrlen;					/* Tamaño de la informacion del cliente */
	int 				client_socketfd;			/* Descriptor de fichero de socket cliente */ 		
	struct sockaddr_in  client_dir;					/* Informacion sobre direccion del cliente */

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
	
	if ( (numPartidas + 1) > MAX_PARTIDAS) {
		fprintf(c.f, "FULL\n");
		printf("[+] Nueva conexión denegada, número máximo de partidas alcanzado\n");
		protocolError(c.f, c.fd);
	} else if (arrPartidas[numPartidas].numJugadores == 0){			

		/* Nueva partida */ 
		partida p;
		p.START_FLAG = FALSE;
		p.PLAYING_FLAG = FALSE;
		p.END_FLAG = FALSE;
		p.TIE_FLAG = FALSE;
		p.tablero = (int**)malloc(nFilas * sizeof(int*));
		for (int j = 0; j < nColumnas; ++j){
			p.tablero[j] = (int*)malloc(nColumnas * sizeof(int));
		}
		for (int i = 0; i < nFilas; ++i)
			for (int j = 0; j < nColumnas; ++j)
				p.tablero[i][j] = 0;

		arrPartidas[numPartidas] = p;
	}
	arrPartidas[numPartidas].numJugadores++;

	fprintf(c.f, "WELCOME\n");

	char cmd[MAXDATASIZE];

	if(fgets(buffer, MAXDATASIZE, c.f) == 0){
		perror("fgets failed");
		protocolError(c.f, c.fd);
	}

	sscanf(buffer, "%s", cmd);
	if( strcmp("REGISTRAR", cmd) == 0)
		registrar(c);
	else if( strcmp("LOGIN", cmd) == 0)	
		login(c, buffer);
	else
		protocolError(c.f, c.fd);
		
}


void login(jugador c, char buffer[MAXDATASIZE]){

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
	
	if(arrPartidas[numPartidas].numJugadores == 1){
		c.player = 1;
		arrPartidas[numPartidas].Jugadores[0] = c;
		printf("[+] Matchmaking. Wait a momemnt man...\n");
	}else if(arrPartidas[numPartidas].numJugadores == 2){
		c.player = -1;
		arrPartidas[numPartidas].Jugadores[1] = c;
		arrPartidas[numPartidas].START_FLAG = TRUE;
		printf("[+] La partida %d esta apunto de comenzar.\n", numPartidas);
		numPartidas++;
	}
}

void registrar(jugador c){

	char buffer[MAXDATASIZE];
	char cmd[MAXDATASIZE];
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
	
	if (arrPartidas[numPartidas].numJugadores == 1) {
		c.player = 1;
		arrPartidas[numPartidas].Jugadores[0] = c;
		printf("[+] Matchmaking...\n");
	} else if(arrPartidas[numPartidas].numJugadores == 2) {
		c.player = -1;
		arrPartidas[numPartidas].Jugadores[1] = c;
		arrPartidas[numPartidas].START_FLAG = TRUE;
		printf("[+] La partida %d esta apunto de comenzar.\n", numPartidas);
		numPartidas++;
		printf("[+] Numero de partidas en juego: %d\n", numPartidas); 
	} 
}


void salir(int i, int j)
{

	printf("[+] Partida %d interrumpida.\n", i);
   	printf("[+] Jugador %s ha cerrado la conexión\n", arrPartidas[i].Jugadores[j].nombre);
   
   	if(j == 1)
   		fprintf(arrPartidas[i].Jugadores[j - 1].f, "SALIR\n");
	else if(j == 0)
		fprintf(arrPartidas[i].Jugadores[j + 1].f, "SALIR\n");
	
	clearPartida(i);
}

void finPartida(int p_index, int j_index){

	if (arrPartidas[p_index].TIE_FLAG == TRUE) {
		fprintf(arrPartidas[p_index].Jugadores[0].f, "TIE\n");
		fprintf(arrPartidas[p_index].Jugadores[1].f, "TIE\n");
		printf("[+] La partida %d ha finalizado\n", p_index + 1);
		clearPartida(p_index);
	}else{
		if (j_index == 0) {
			fprintf(arrPartidas[p_index].Jugadores[j_index].f, "VICTORY\n");
			fprintf(arrPartidas[p_index].Jugadores[j_index + 1].f, "DEFEAT\n");
			printf("[+] La partida %d ha finalizado\n", p_index + 1);
			clearPartida(p_index);
		} else if(j_index == 1) {
			fprintf(arrPartidas[p_index].Jugadores[j_index].f, "VICTORY\n");
			fprintf(arrPartidas[p_index].Jugadores[j_index - 1].f, "DEFEAT\n");
			printf("[+] La partida %d ha finalizado\n", p_index + 1);
			clearPartida(p_index);
		}
	}
}

void comienzaPartida(int i, int nFilas, int nColumnas){

	fprintf(arrPartidas[i].Jugadores[0].f, "START %s %d %d\n", arrPartidas[i].Jugadores[1].nombre, nFilas, nColumnas);
	fprintf(arrPartidas[i].Jugadores[1].f, "START %s %d %d\n", arrPartidas[i].Jugadores[0].nombre, nFilas, nColumnas);
	
	/* Quien empieza jugando ? */
	time_t t;
	srand((unsigned) time(&t));
	bool x = rand() & 1;
	if(x){
		fprintf(arrPartidas[i].Jugadores[0].f, "URTURN\n");
		printf("[+] %s empiezas jugando.\n", arrPartidas[i].Jugadores[0].nombre);					
		arrPartidas[i].Jugadores[0].player = 1;
		arrPartidas[i].Jugadores[1].player = -1;
	}else{
		fprintf(arrPartidas[i].Jugadores[1].f, "URTURN\n");
		printf("[+] %s empiezas jugando.\n", arrPartidas[i].Jugadores[1].nombre);
		arrPartidas[i].Jugadores[0].player = -1;
		arrPartidas[i].Jugadores[1].player = 1;
	}
	printf("[+] Simbolo de %s = %d\n", arrPartidas[i].Jugadores[0].nombre, arrPartidas[i].Jugadores[0].player);
	printf("[+] Simbolo de %s = %d\n", arrPartidas[i].Jugadores[1].nombre, arrPartidas[i].Jugadores[1].player);

	for (int z = 0; z < nFilas; ++z)
		for (int j = 0; j < nColumnas; ++j)
			arrPartidas[i].tablero[z][j] = 0;

	arrPartidas[i].START_FLAG = FALSE;	
	arrPartidas[i].PLAYING_FLAG = TRUE;

}

void mostrarTablero(int nFil, int nCol, int **tablero){

	for (int i = 0; i < nCol; ++i)
		printf("  %d \t", i);
	printf("\n");

    for (int i = 0; i < nFil; ++i){
    	for (int j = 0; j < nCol; ++j){
    		printf("| %d |\t", tablero[i][j]);
    	}
    	printf("\n");
    }
    printf("\n\n");
}

void turno(int p_index, int j_index, int tempColumna){
	
	if (j_index == 0) {
		fprintf(arrPartidas[p_index].Jugadores[j_index + 1].f, "URTURN %d\n", tempColumna);
		printf("[+] Es el turno de %s.\n", arrPartidas[p_index].Jugadores[j_index + 1].nombre);
	} else if (j_index == 1) {
		fprintf(arrPartidas[p_index].Jugadores[j_index - 1].f, "URTURN %d\n", tempColumna);
		printf("[+] Es el turno de %s.\n", arrPartidas[p_index].Jugadores[j_index - 1].nombre);
	}

}
			

