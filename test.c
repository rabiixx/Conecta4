#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char const *argv[]){
	
	int col;
	char matrix[6][6];
	for (int i = 0; i < 6; ++i)
		for (int j = 0; j < 6; ++j)
			matrix[i][j] = 'A';
	matrix[5][5] = 'O';
	matrix[4][5] = 'O';

	printf("Introduce una columa: \n");
	scanf("%d", &col);
	printf("%d\n",col );
	int z = 0; 
	while(z < 6){
		if(matrix[z][col] != 'A')
			break;
		z++;
	}
	printf("%d\n", (z - 1));
	matrix[z - 1][col] = 'O';

	for (int i = 0; i < 6; ++i){
		for (int j = 0; j < 6; ++j)
			printf("%c\t", matrix[i][j]);
		printf("\n");
	}



	return 0;
}