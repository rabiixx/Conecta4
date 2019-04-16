#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
#define pi 3.142857



int matrix[6][6] = {
					{1, 0, 0, 1 , -1, -1},
					{1, 0, 0, 1 , -1, -1},
					{1, 0, 0, 1 , -1, -1},
					{1, 0, 0, 1 , -1, -1},
					{1, 0, 0, 1 , -1, -1},
					{1, 0, 0, 1 , -1, -1}};


int i,j,k;

void display2 (void)  
{ 
    glClear(GL_COLOR_BUFFER_BIT); 
    glBegin(GL_POINTS); 
    float x, y, i; 
      
    // iterate y up to 2*pi, i.e., 360 degree 
    // with small increment in angle as 
    // glVertex2i just draws a point on specified co-ordinate 
    for ( i = 0; i < (2 * pi); i += 0.001) 
    { 
        // let 200 is radius of circle and as, 
        // circle is defined as x=r*cos(i) and y=r*sin(i) 
        x = 200 * cos(i); 
        y = 200 * sin(i); 
          
        glVertex2i(x, y); 
    } 
    glEnd(); 
    glFlush(); 
} 
void display(void){
	/*glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_POINTS); 
	int x, y;

	for(i = 0;i < 6;i++ ){
		for(j = 0;j < 6;j++ ){
			if(matrix[i][j] == 1){
				//glColor3f(1, 0, 0);
				for ( int z = 0; z < (2 * pi); z += 0.001){ 
				    x = 20 * cos(z); 
				    y = 20 * sin(z); 
				        
				    glVertex2i(x, y); 
				}  
			}else if(matrix[j][i] == -1){
				for ( int z = 0; z < (2 * pi); z += 0.001){ 
				    x = 20 * cos(z); 
				    y = 20 * sin(z); 
				        
				    glVertex2i(x, y); 
				}
				//glBegin(GL_QUADS);
				//glColor3f(0, 1, 0);
			}else{
				//glBegin(GL_QUADS);
				//glColor3f(1, 1,1 );
				for ( int z = 0; z < (2 * pi); z += 0.001){ 
				    x = 20 * cos(z); 
				    y = 20 * sin(z); 
				        
				    glVertex2i(x, y); 
				}
			}
			glVertex2f(i,j);
			glVertex2f(i+1,j);
			glVertex2f(i+1,j+1);
			glVertex2f(i,j+1);
			glEnd();
 		}
 	}

 	glFlush ();*/
 	glClear (GL_COLOR_BUFFER_BIT);
	for(i = 0;i < 8;i++ ){
	 	for(j = 0;j < 8;j++ ){

			if((i%2) == 0){
				if((j%2) == 0){
					glutDisplayFunc(display2);
				}else{
					glutDisplayFunc(display2);
			 	}
			}else {
				if((j%2) == 0){
					glutDisplayFunc(display2);
			 	}else{
			 		glutDisplayFunc(display2);
			 	}
			}

		 /*glVertex2f (i,j);
		 glVertex2f (i+1,j);
		 glVertex2f (i+1,j+1);
		 glVertex2f (i,j+1);
		 glEnd();*/
		 }
	 }
	 glFlush ();
}



void init (void){

	// making background color black as first  
	glClearColor (0.0 , 0.0 , 0.0 , 0.0 );
 	glColor3f(0.0, 1.0, 0.0);
 	glPointSize(100); 
 	glMatrixMode(GL_PROJECTION);
 	glLoadIdentity();
 	//gluOrtho2D(-780, 780, -420, 420);
 	glOrtho(0, 8, 0, 8, -1.0, 1.0);

}

int main(int argc, char** argv){
	
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
	
	// giving window size in X- and Y- direction 
	glutInitWindowSize (900, 600);
	glutInitWindowPosition (0, 0);
	
	// Giving name to window 
	glutCreateWindow ("Chess Board");
	init ();
	 
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}


