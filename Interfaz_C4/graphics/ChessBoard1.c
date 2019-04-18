#include <stdio.h>
#include <curses.h>
#include <graphics.h>




void main(){

	if(argc != 3){
		fprintf(stderr, "%s <numero filas> <numero columnas> \n", argv[1]);
		exit(EXIT_SUCCESS);
	}

	int nFil = atoi(argv[1]);	/* Numero de columnas del tablero */
	int nCol = atoi(argv[2]);	/* Numero de columnas del tablero */

	int matrix[nFil][nCol];

	matrix[nFil][nCol] = {
					{1, 0, 0, 1 , -1, -1},
					{1, 0, 0, 1 , -1, -1},
					{1, 0, 0, 1 , -1, -1},
					{1, 0, 0, 1 , -1, -1},
					{1, 0, 0, 1 , -1, -1},
					{1, 0, 0, 1 , -1, -1}
				};


	/* gm is Graphics mode which is  a computer display mode that 
    * generates image using pixels. DETECT is a macro defined in 
    * "graphics.h" header file */
	int gr = DETECT,gm;
	int xRec = 50, yRec = 50; 
	int radio = 25;
	int row, col;
	int x, y;		/* Coordenadas d(x, y) del centro del circulo  */
	initgraph(&gr,&gm,"C:\\TURBOC3\\BGI");
	printf("\t ***|CONECTA 4|***\n");
	
	for(row = 0; row < 8; row++){
		x = 50;
		for(col = 1; col <= nCol; col++){
			if(matrix[i][j] == 1){
				setcolor(GREEN);
				setfillstyle(SOLID_FILL, RED);				 
   				circle( x + radio, y + radio, radio); 
				floodfill(x + 1, y + 1, GREEN);
			}else if(matrix[i][j] == -1){
				setcolor(GREEN);
				setfillstyle(SOLID_FILL, BLUE);
   				circle( x + radio, y + radio, radio); 
				floodfill(x + 1, y + 1, GREEN);
			}else{
				setcolor(GREEN);
				setfillstyle(SOLID_FILL, BLACK);
				rectangle(xRec, yRec, x + (2 * radio), y + (2 * radio));
				floodfill(x + 1, y + 1, GREEN);
			}
			x += 2 * radio;
		}
		y += 2 * radio;
		delay(100);
	}
	
	getch();
	closegraph();
}