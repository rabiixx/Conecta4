#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


int simulador(int R, int C, char **tablero);
int mejorJugada(int casosOk[][3], int casosNOok[][3]);
int prometedorCol(int C, char **tablero, int j,int i,char fichaJug);
int prometedorFila(int C, char **tablero, int i, int j,char fichaJug);
int prometedorDiagonal(int C, char **tablero, int i, int j, char dir, char fichaJug);
int analizaEstados(int C, int R, char **tablero, char fichaBot, char fichaJug, int jugadas[][3],char Usuario);
int intro();


int simulador(int R, int C, char **tablero)
{

	char fichaBot      = 'X';
    char fichaJugador  = 'O';

	int casosOk[4][3];
	analizaEstados(C, R, tablero,fichaBot,fichaJugador,casosOk,'B');//El primer carater es la ficah del bot el segundo caracter es la ficaha del jugador

	int casosNOok[4][3];
	analizaEstados(C, R, tablero,fichaJugador,fichaBot,casosNOok,'J');

	return mejorJugada(casosOk,casosNOok);
}

int analizaEstados(int C, int R, char **tablero, char fichaBot, char fichaJug, int jugadas[][3],char Usuario){//buscamos las posibles combianciones de ganar
	int vaFin = 0;
  	int auxpos;
	for (int i=0; i < C; i++){
      if (tablero[R-1][i] != fichaBot && tablero[R-1][i] != fichaJug){
	        vaFin++;
	    }
      if (tablero[R-1][i] == fichaJug){
        auxpos = i;
	    }
	}
  	if (vaFin == C-1){
	    for (int i = 0; i < 4; i++){
	        int x;
	            jugadas[i][0] = ((x = rand() % (C/2+1)) != auxpos) ? (x) :  (rand() % (C/2+1));
	            jugadas[i][1] = 1700;
	            jugadas[i][2] = 1700;
	  	}
	    return 1;
  	}

	//Buscar por columnas
	int posCol;
	int numColFich = 0; //numeor de ficahs faborable en la jugada
	for (int j = 0; j < C; j++){
		for (int i = 0; i < R-3; i++){
			if (prometedorCol(C, tablero,j,i,fichaJug)){

				int numColFichaux = 0;
				for  (int x = i; x < i+4; x++){
					if (tablero[x][j] == fichaBot)
						numColFichaux++;
				}

				if (numColFich <= numColFichaux){
							posCol = j;
							numColFich = numColFichaux;
				}
			}
		}
	}

	//Buscar por filas
	int posFila;// columna pra introducir la ficah de la mejor jugada del escaneo por columnas
	int minFil = 35000;//guardamos le numeorminimo de oviemientos para realizar la jugada
	int numFilFich = 0; //numeor de ficahs faborable en la jugada
	for (int i = 0; i < R; i++ ){
		for (int j = 0; j < C-3; j++ ){
			if (prometedorFila(C, tablero,i,j,fichaJug) ){
				int numFilFichaux = 0;
				for  (int y = j; y < j+4; y++){
					if (tablero[i][y] == fichaBot)
						numFilFichaux++;
				}

				int numMov;
				for (int y = j; y < j+4; y++){
					numMov = 0;
					for (int x = i; x < R; x++)
						if (tablero[x][y] != fichaJug && tablero[x][y] != fichaBot){
							numMov++;
						}

					if (numMov % 2 == 1 && Usuario == 'B' ){
						if (numFilFich <= numFilFichaux){
							if (numMov <= 3){
								posFila = y;
								minFil = numMov;
								numFilFich = numFilFichaux;
							}
						}
					}else if (Usuario == 'J' && numMov != 0){
							if (numFilFich <= numFilFichaux){
								if (numMov <= 3){
									posFila = y;
									minFil = numMov;
									numFilFich = numFilFichaux;
								}
							}
					}
				}
			}
		}
	}

//Busacr por diagonales
/////////////////////////////
	int posDigDer;//sera el valos de la columan en el que se quiera insetrar la ficha par a la jugada
	int minDigDer = 35000;
	int numFichDer = 0;
	for (int i = 3; i < R; ++i){
		for (int j = 0; j < C-3; ++j){
			if (prometedorDiagonal(C, tablero,i,j,'D', fichaJug) ){
				//Buscamos cauntas ficahs hacen falta para cada completar cada pscion de la diagonal
				//Sera  faborable si el numero de fiuchas par una poscion es impar o 1
				int numMov;
				int numFichaux;
				for (int x = 0; x <= 3; x++){//se compreuba una posible diagonal y se busca la ficaha que mjor staisfagaalafuncion fitness
					numMov = 0;
					numFichaux = 0;
					for (int pos = i-x; pos < R; pos++){//recorremos las columnas
						if (tablero[pos][j+x] != fichaJug && tablero[pos][j+x] != fichaBot){
							numMov++;
						}
					}
					for (int x = 0; x <= 3; x++){
						if (tablero[i-x][j+x] == fichaBot)
							numFichaux++;
					}

					if (numMov % 2 == 1 && Usuario == 'B'){
						if (numFichaux >= numFichDer)
							if (numMov <= minDigDer || numFichaux >= 3){
								minDigDer = numMov;
								posDigDer = j+x;
								numFichDer =numFichaux;
							}
					}else if (Usuario == 'J' && numMov != 0){
						if (numFichaux >= numFichDer)
							if (numMov <= minDigDer || numFichaux >= 3){
								minDigDer = numMov;
								posDigDer = j+x;
								numFichDer =numFichaux;
							}
					}
				}
			}
		}
	}

////////////////////////////
	int posDigIzq;//sera el valos de la columan en el que se quiera insetrar la ficha par a la jugada
	int minDigIzq = 35000;
	int numFichIzq = 0;
	for (int i = 0; i < R-3; ++i){
		for (int j = 0; j < C-3; ++j){
			if (prometedorDiagonal(C, tablero,i,j,'I', fichaJug) ){
				//Buscamos cauntas ficahs hacen falta para cada completar cada pscion de la diagonal
				//Sera  faborable si el numero de fiuchas par una poscion es impar o 1
				int numMov;
				int numFichaux;
				for (int x = 0; x <= 3; x++){//se compreuba una posible diagonal y se busca la ficaha que mjor staisfagaalafuncion fitness
					numMov = 0;
					numFichaux = 0;
					for (int pos = i+x; pos < R; pos++){//recorremos las columnas
						if (tablero[pos][j+x] != fichaJug && tablero[pos][j+x] != fichaBot){
							numMov++;
						}
					}
					for (int x = 0; x <= 3; x++){
						if (tablero[i+x][j+x] == fichaBot)
							numFichaux++;
					}

					if ((numMov % 2 == 1) && (Usuario == 'B')){
						if (numFichaux >= numFichIzq)
							if (numMov <= minDigIzq || numFichaux >= 3){
								minDigIzq = numMov;
								posDigIzq = j+x;
								numFichIzq =numFichaux;
							}
					}else if ((Usuario == 'J') && numMov != 0){
							if (numFichaux >= numFichIzq)
								if (numMov <= minDigIzq || numFichaux >= 3){
									minDigIzq = numMov;
									posDigIzq = j+x;
									numFichIzq =numFichaux;
								}
					}
				}
			}
		}
	}

	jugadas[0][0] = posCol;
	if (numColFich == 3)
		jugadas[0][1] = 1;
	else
		jugadas[0][1] = 2;
	jugadas[0][2] = numColFich;

	jugadas[1][0] = posFila;
	jugadas[1][1] = minFil;
	jugadas[1][2] = numFilFich;

	jugadas[2][0] = posDigDer;
	jugadas[2][1] = minDigDer;
	jugadas[2][2] = numFichDer;

	jugadas[3][0] = posDigIzq;
	jugadas[3][1] = minDigIzq;
	jugadas[3][2] = numFichIzq;


	int vacios = 0;
	for (int i=0; i < C; i++){
	    if (tablero[0][i] != fichaBot && tablero[0][i] != fichaJug){
	        vacios++;
	    }
	}

	int posibles[vacios];
	int j = 0;
	for (int i=0; i < C; i++){
	    if (tablero[0][i] != fichaBot && tablero[0][i] != fichaJug){
	        posibles[j] = i;//guardamos los valores que son candidatos para la jugada
	        j++;
	    }
	}
    //Comprobacion
	//Hay que que evitar los caso en los que no  se haya conseguido sacar ninguna jugada posible
	//y rellena r la matriz con esoooo
	int comp = 0;
	for (int i = 0; i < 4; i++){
        for (int x = 0; x < vacios; x++){
            if (jugadas[i][0] == posibles[x]){
                comp = 1;
            }
        }
        if (comp == 0){
            jugadas[i][0] = posibles[rand() % vacios];
            jugadas[i][1] = 1700;
            jugadas[i][2] = 1700;
        }
        comp = 0;
	}
	return 1;
}

int prometedorDiagonal(int C, char **tablero, int i, int j, char dir,char fichaJug){
	if (dir == 'D'){
		for (int x = 0; x <= 3; x++){
			if (tablero[i-x][j+x] == fichaJug)
				return 0;
		}
		return 1;
	}else if (dir == 'I'){
		for (int x = 0; x <= 3; x++){
			if (tablero[i+x][j+x] == fichaJug)
				return 0;
		}
		return 1;
	}
	return 0;
}

int prometedorFila(int C, char **tablero, int i,int j,char fichaJug){
	int count = 0;
	for  (int y = j; y < j+4; y++){
		if (tablero[i][y] == fichaJug){
			count = 0;
		}else
			count++;
	}
	if (count > 3)
		return 1;
	else
		return 0;
}

int prometedorCol(int C, char **tablero, int j,int i,char fichaJug){
	int count = 0;
	for  (int x = i; x < i+4; x++){
		if (tablero[x][j] == fichaJug){
			count = 0;
		}else
			count++;
	}
	if (count > 3)
		return 1;
	else
		return 0;
}

int mejorJugada(int casosOk[][3], int casosNOok[][3]){
	for (int i = 0; i < 4; i++){
		if (casosNOok[i][1] == 1 && casosNOok[i][2] == 3){
			return casosNOok[i][0];
		}
	}
	for (int i = 0; i < 4; i++){
		if (casosOk[i][1] == 1 && casosOk[i][2] == 3){
			return casosOk[i][0];
		}
	}
	for (int i = 0; i < 4; i++){
		if (casosNOok[i][1] == 1 && casosNOok[i][2] >= 2){
			return casosNOok[i][0];
		}
	}
	for (int i = 0; i < 4; i++){
		if (casosOk[i][1] == 1 && casosOk[i][2] >= 2){
			return casosOk[i][0];
		}
	}
	for (int i = 0; i < 4; i++){
		if (casosOk[i][1] == 1 && casosOk[i][2] > 0){
			return casosOk[i][0];
		}
	}
	return casosOk[rand() % 4][0];
}

int intro(){
	printf("\n");
	printf("***********************************************************************************************************************************\n");
	printf("*********************************************************  JC SOUL  ***************************************************************\n");
	printf("***********************************************************************************************************************************\n");
	printf("**                                                                                                                               **\n");
	printf("**      *******      ********     **            **   *************       ********   **************       **          **      **  **\n");
	printf("**    **           **        **   ****          **   ************      **           **************      ****         **      **  **\n");
	printf("**   **           **          **  ** **         **   **               **                  **           **  **        **      **  **\n");
	printf("**  **           **           **  **   **       **   *****+          **                   **          **    **       **      **  **\n");
	printf("**  **           **           **  **     **     **   *****+          **                   **         **      **      **********  **\n");
	printf("**   **           **         **   **       **   **   **               **                  **        ************             **  **\n");
	printf("**    **           **       **    **         ** **   ************      **                 **       **          **            **  **\n");
	printf("**     *******       *******      **            **   *************      ********          **      **            **           **  **\n");
	printf("**                                                                                                                               **\n");
	printf("***********************************************************************************************************************************\n");
	printf("********************************************************* CONNECT 4 ***************************************************************\n");
	printf("***********************************************************************************************************************************\n");
	printf("\n");
	printf("El juego no implementa la comprobacion de si hay 4 en linea\n");
	printf("\n");
	return 1;
}
