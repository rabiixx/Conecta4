//#include <windows.h>
#ifdef __APPLE__
#else
#include <GL/glut.h>
#endif

//========= main background  section color ===========//
void init(void){
    glClearColor (0.20, 0.40, 0.10, 0.90);
}

void display(){
    glClearColor(0.50, 0.50, 0.50, 0.50 );

    //============== My border section ===================//
    glLineWidth(5);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
    glLineWidth(5.50);
    glColor3f(0.0 , 0.0 , 0.0);
    glBegin(GL_LINES);
    glVertex3f(90, 90, 0);
    glVertex3f(720, 90, 0);

    glVertex3f(720, 90, 0);
    glVertex3f(720, 720, 0);

    glVertex3f(720, 720, 0);
    glVertex3f(90, 720, 0);

    glVertex3f(90, 720, 0);
    glVertex3f(90, 90, 0);
    glEnd();

//====================== MY loop section ===============//
  int x,y,color=150;
    int sh=8;
    for(x=1; x<=sh; x++){
        if(color == 0){
           glColor3f(0.3, 0.7  , 0.9 );
            color++; 
        }else{
            glColor3f (1.0, 1.0, 1.0);
            color=0;
        }

        for(y=1; y<=sh; y++){
            if(color==0){ glColor3f(0.3, 0.1  , 0.9 );
                color++;
            }else{ 
                glColor3f (9.0, 0.1, 0.1); color=0;
            }

            glBegin(GL_QUADS);
            glVertex2f(90*x, 90*y);   // 80, 80,
            glVertex2f(90+90*x, 90*y); //160, 80
            glVertex2f(90+90*x, 90+90*y); //160, 160
            glVertex2f(90*x, 90+90*y);  // 80, 160
            glEnd();
        }
    }

    glFlush ();
}

//=============== my display section mathod  ==========//
void reshape (int w, int h){
    glViewport (1.0, 1.0, w, h);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    gluOrtho2D (1.0, w, 1.0, h);
}

//=============== Some commone issue DOOOOOOOm =========//

int main(int argc, char** argv){
    
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (1000, 1000);
    glutInitWindowPosition (10, 10);
    glutCreateWindow ("Shakil Hossain id :152-15-6306");
    init ();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}