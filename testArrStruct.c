
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>



typedef struct _partida{
	int START_FLAG;
	int PLAYING_FLAG;
	int END_FLAG;
	int numJugadores;
}partida;

int main(int argc, char const *argv[]){

	partida arr[6];
	partida p_inic;
	p_inic.START_FLAG = 0;
	p_inic.PLAYING_FLAG = 0;
	p_inic.END_FLAG = 0;
	p_inic.numJugadores = 0;

	for (int i = 0; i < 6; ++i){
		arr[i] = p_inic;
	}


	for (int i = 0; i < 6; ++i){
		if(arr[i].numJugadores == 0)
			printf("hola\n");
		else
			printf("adios\n");
	}

	return 0;
}