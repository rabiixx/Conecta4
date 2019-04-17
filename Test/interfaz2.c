#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>


int main(void){
	
	time_t t;
	srand((unsigned) time(&t));
	bool x = rand() & 1; 

	char tablero[6][6] ={
		{'A', 'A', 'X', 'X', 'A', 'O'},
		{'A', 'A', 'X', 'X', 'A', 'O'},
		{'A', 'A', 'X', 'X', 'A', 'O'},
		{'A', 'A', 'X', 'X', 'A', 'O'},
		{'A', 'A', 'X', 'X', 'A', 'O'},
		{'A', 'A', 'X', 'X', 'A', 'O'},
	}

	printf("%s\n", x ? "true":"false");
	

	for (int i = 0; i < 6; ++i){
	  	for (int j = 0; j < 6; ++j){
	  		if(tablero[i][i] == 'A'){
	  			printf(" ___\n");
	  			printf("|   |\n");
	  			printf(" ___\n");
	  		}else if(tablero[i][i] == 'X'){
				printf(" ___\n");
		   		printf("| X |\t", tablero[i][j]);
		   		printf(" ___\t");
		   	} else if(tablero[i][i] == 'O')
				printf(" ___\n");
		   		printf("| 0 |\t", tablero[i][j]);
		   		printf(" ___\t");
	   	}
	   	printf("\n");
	}


	return 0;
}


