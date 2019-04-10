#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define TRUE 1
#define FALSE 0 

int connect4(int maxRow, int maxCol, int tablero[][maxCol], int player){

    // horizontalCheck 
    for (int j = 0; j<maxRow-3 ; j++ ){
        for (int i = 0; i<maxCol; i++){
            if (tablero[i][j] == player && tablero[i][j+1] == player && tablero[i][j+2] == player && tablero[i][j+3] == player){
                return TRUE;
            }           
        }
    }
    // verticalCheck
    for (int i = 0; i<maxCol-3 ; i++ ){
        for (int j = 0; j<maxRow; j++){
            if (tablero[i][j] == player && tablero[i+1][j] == player && tablero[i+2][j] == player && tablero[i+3][j] == player){
                return TRUE;
            }           
        }
    }
    // ascendingDiagonalCheck 
    for (int i=3; i<maxCol; i++){
        for (int j=0; j<maxRow-3; j++){
            if (tablero[i][j] == player && tablero[i-1][j+1] == player && tablero[i-2][j+2] == player && tablero[i-3][j+3] == player)
                return TRUE;
        }
    }
    // descendingDiagonalCheck
    for (int i=3; i<maxCol; i++){
        for (int j=3; j<maxRow; j++){
            if (tablero[i][j] == player && tablero[i-1][j-1] == player && tablero[i-2][j-2] == player && tablero[i-3][j-3] == player)
                return TRUE;
        }
    }

    return FALSE;



}

int main(int argc, char const *argv[]){

    int TIE_FLAG = TRUE;   
    int maxRow = 6;
    int maxCol = 6;
    int tablero[6][6] = {
                                {0, 0, 0, 0, 0, 0}, 
                                {0, 1, 0, 0, 0, 0},
                                {1, 1, 0, 0, 1, 0},
                                {0, 0, 0, 0, 0, 0},
                                {0, 0, 1, 0, 0, 0},
                                {0, 0, 0, 1, -1, 0}
                                };
    
    /*for (int i = 0; i < maxRow; ++i)
        for (int j = 0; j < maxCol; ++j)
            tablero[i][j] = 0;*/

    const int player1 = 1;
    const int player2 = 2;

    if(connect4(maxRow, maxCol, tablero, player1) == TRUE){
        printf("Player 1 Victory!\n");
    }else if(connect4(maxRow, maxCol, tablero, player1) == TRUE){
        printf("Player 2 Victory!\n");
    }else{
        for (int i = 0; i < 6; ++i){
            for (int j = 0; j < 6; ++j){
                if(tablero[i][j] == -1){
                    TIE_FLAG = FALSE;
                    break;
                }
            }
            //break;
        }
    }
    if(TIE_FLAG == FALSE){
        printf("la partida aun no ha acabado\n");
    }else{
        printf("TIE\n");
    }

}

