
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char const *argv[])
{	
	
	int nFil = 3;
	int nCol = 3;

	int **matrix;

	matrix = (int**)malloc(3 * sizeof(int*));
	for (int i = 0; i < 3; ++i){
		matrix[i] = (int*)malloc(3 * sizeof(int));
	}

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			matrix[i][j] = 5;

	for (int i = 0; i < 3; ++i){
		for (int j = 0; j < 3; ++j)
			printf("%d\t", matrix[i][j]);	
		printf("\n");
	}


	return 0;
}


