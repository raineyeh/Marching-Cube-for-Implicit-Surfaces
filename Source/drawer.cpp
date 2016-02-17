#include "drawer.h"


#include "glut.h"

/* Window information */
float windowWidth = 800;
float windowHeight = 600;
int windowID = -1;



void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(2.5);
	glColor3f(.5, 0.5, 0.5);
	glBegin(GL_POLYGON);
	glVertex2f(-.2, -.5);
	glVertex2f(-.2, .5);
	glVertex2f(.5, 0);
	glEnd();

	glFlush();
	glutSwapBuffers();
}


void idle()
{
	glutPostRedisplay();
}


void init()
{
	/* Set clear color */
	glClearColor(1.0, 1.0, 1.0, 0.0);

	/* Set 2D orthogonal projection */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1.0, 1.0, 1.0, -1.0);
}



Drawer::Drawer(int* argc, char** argv){

	/* Initialize the GLUT window */
	glutInit(argc, argv);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(30, 30);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	windowID = glutCreateWindow("Lalala");

	/* Set OpenGL initial state */
	init();

	/* Callback functions */
	glutDisplayFunc(display);
	glutIdleFunc(idle);


}

bool Drawer::set_march(Marching* m){
	marching_obj = m;
	return true;
}

void Drawer::start(){

	/* Start the main GLUT loop */
	/* NOTE: No code runs after this */
	glutMainLoop();
}