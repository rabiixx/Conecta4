#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	
	int tablero[6][6] = {
                        {0, 0, 0, 0, 0, 0}, 
                        {0, 1, 0, 0, 0, 0},
                        {1, 1, 0, 0, 1, 0},
                        {0, 0, 0, 0, 0, 0},
                        {0, 0, 1, 0, 0, 0},
                        {0, 0, 0, 1, -1, 0}
                        };


    for (int i = 0; i < 6; ++i)
    {
    	for (int j = 0; j < 6; ++j)
    	{
    		printf("| %d |\t", tablero[i][j]);
    	}
    	printf("\n");
    }

	return 0;
}