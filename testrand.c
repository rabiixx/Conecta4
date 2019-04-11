#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

int main(void){
	
	bool x = rand()>(RAND_MAX/2);

	printf("%s", x ? "true":"false");
	return 0;
}


