#include "drawer.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "imgui_impl_glut.h"

/* Window information */
float windowWidth = 800;
float windowHeight = 600;
int windowID = -1;


void draw_gui()
{	
	ImGui_ImplGlut_NewFrame("Marching Cube");
	ImVec2 wsize(400.0f, 150.0f);
	ImGui::SetWindowFontScale(1.5);
	ImGui::SetWindowSize(wsize);
	char buf[256] = "x^2 + y^2 = 0";
	static int grid = 16.0f;
	ImGui::InputText("Polynomial", buf, 256);
	ImGui::SliderInt("Grid size", &grid, 16.0f, 128.0f);

	if (ImGui::Button("Refresh"))
	{
		int a = 0;
	}
	//    int blur_loc = glGetUniformLocation(shader_program, "grid");
	// 	  glUniform1i(blur_loc, grid); 

	ImGui::Render();
}
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

	draw_gui();

	glFlush();
	glutSwapBuffers();
}

// glut callbacks need to send keyboard and mouse events to imgui
void keyboard(unsigned char key, int x, int y)
{
	ImGui_ImplGlut_KeyCallback(key);	
}

void keyboard_up(unsigned char key, int x, int y)
{
	ImGui_ImplGlut_KeyUpCallback(key);
}

void special_up(int key, int x, int y)
{
	ImGui_ImplGlut_SpecialUpCallback(key);
}

void passive(int x, int y)
{
	ImGui_ImplGlut_PassiveMouseMotionCallback(x, y);
}

void special(int key, int x, int y)
{
	ImGui_ImplGlut_SpecialCallback(key);
}

void motion(int x, int y)
{
	ImGui_ImplGlut_MouseMotionCallback(x, y);
}

void mouse(int button, int state, int x, int y)
{
	ImGui_ImplGlut_MouseButtonCallback(button, state);
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
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutKeyboardUpFunc(keyboard_up);
	glutSpecialUpFunc(special_up);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);

	glutIdleFunc(idle);
	glewInit();
	
	ImGui_ImplGlut_Init();	
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