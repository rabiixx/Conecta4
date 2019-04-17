#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char const *argv[])
{

	int arr[10] = {0, 1, 0, 3, 1, 0, 0, 0, 0, 0};

	int i;
	for (i = 0; i < 10; ++i){
		if(arr[i] == 2)
			break;
	}

	printf("%d\n", i);

	return 0;
}