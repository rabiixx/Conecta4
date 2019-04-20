/* conecta4_bot.h */

int simulador(int R, int C, char tablero[][C]);
int mejorJugada(int casosOk[][3], int casosNOok[][3]);
int prometedorCol(int C, char tablero[][C], int j,int i,char fichaJug);
int prometedorFila(int C, char tablero[][C], int i, int j,char fichaJug);
int prometedorDiagonal(int C, char tablero[][C], int i, int j, char dir, char fichaJug);
int analizaEstados(int C, int R, char tablero[][C], char fichaBot, char fichaJug, int jugadas[][3],char Usuario);
int intro();