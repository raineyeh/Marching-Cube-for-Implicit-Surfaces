/* This file replaces the drawer UI to display marching cube surface graphically.
It's used in development testing and not in the final product. 

To use, call test_marching(Marching* m) in main with an equation.
eg.

int main(int argc, char **argv) 
{
	Evaluator evaluator;
	evaluator.set_equation("x^2+y^2-0.5");
	Marching march_maker;
	march_maker.set_evaluator(&evaluator);
	test_marching(&march_maker);
	return 0;
}

controls are printed in console
*/

#include <iostream>
#include <windows.h>
#include "GL/glut.h"
#include "marching.h"

bool show_steps = true;
float surface_constant = 0.0;
bool using_constraint_0 = false;
bool use_seed_mode = true;
float scaling_x = 1.0;

using namespace std;

/* Window information */
float m_test_windowWidth = 600;
float m_test_windowHeight = 600;
int m_test_windowID = -1;

// angle of rotation for the camera direction
float angle = 0.0;
float eyex = 0.0f, eyey = 10.0f;


/* Data information */
const Poly_Data* poly_data = NULL;
Marching *M;


void m_test_display()
{

	glClear(GL_COLOR_BUFFER_BIT); 
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(15.0, 1.0, 8.0, 12.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyex, 0, eyey, 0, 0, 0, 0.0f, 1.0f, 0.0f);

	//center red points for reference
	/*
	glPointSize(10);
	glColor3f(1, 0, 0);
	glBegin(GL_POINTS);
	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 0);
	glEnd();
	*/
	
	bool front = true;
	//glPolygonMode(GL_BACK, GL_LINE);
	//glPolygonMode(GL_FRONT, GL_FILL);
	glLineWidth(2.5);
	glColor3f(.2, .2, .8);
	
	if (!poly_data){
		__int64 ctr1 = 0, ctr2 = 0, freq = 0;
		if (QueryPerformanceCounter((LARGE_INTEGER *)&ctr1) != 0) {
		M->recalculate();
		QueryPerformanceCounter((LARGE_INTEGER *)&ctr2);
		QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

		// Print the time spent in microseconds to the console.

		std::cout << "Time information "<< ((ctr2 - ctr1) * 1.0 / freq) << std::endl;
		}
	}
	poly_data = M->get_poly_data();

	for(int draw_i = 0; draw_i < 3; draw_i ++){
		glEnable(GL_CULL_FACE);
		if (draw_i == 2){
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			//glCullFace(GL_FRONT_AND_BACK);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glColor3f(.2, .2, .8);
		}
		else if (draw_i == 0){
			glCullFace(GL_FRONT);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glColor3f(.8, .8, .8);
		}
		else {
			glCullFace(GL_BACK);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glColor3f(.2, .2, .2);
		}
		for (int i = 0; i < poly_data->tri_list.size(); i += 3){
			glBegin(GL_TRIANGLES);
			unsigned int vi1 = poly_data->tri_list[i];
			unsigned int vi2 = poly_data->tri_list[i + 1];
			unsigned int vi3 = poly_data->tri_list[i + 2];
			glVertex3f(poly_data->vertex_list[vi1 * 3], poly_data->vertex_list[vi1 * 3 + 1], poly_data->vertex_list[vi1 * 3 + 2]);
			glVertex3f(poly_data->vertex_list[vi2 * 3], poly_data->vertex_list[vi2 * 3 + 1], poly_data->vertex_list[vi2 * 3 + 2]);
			glVertex3f(poly_data->vertex_list[vi3 * 3], poly_data->vertex_list[vi3 * 3 + 1], poly_data->vertex_list[vi3 * 3 + 2]);
			glEnd();
		}
		
	} 

	if (show_steps && poly_data->step_data.step_i>=0){
		//step points
		glPointSize(8);
		glBegin(GL_POINTS);
		for (int i = 0; i < poly_data->step_data.corner_coords.size(); i += 3){
			if (poly_data->step_data.corner_values[i/3]>poly_data->step_data.surf_constant)
				glColor3f(1, 0, 0);
			else
				glColor3f(0, 1, 0);
			float x = poly_data->step_data.corner_coords[i];
			float y = poly_data->step_data.corner_coords[i + 1];
			float z = poly_data->step_data.corner_coords[i + 2];

			glVertex3f(x, y, z);
		}
		glEnd();
		
		//step triangles
		
		for (int m = 0; m < 2; m++){
			if (m==0){
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glColor3f(1, 1, 0);
			}
			else{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(.2, .2, .8);
			}
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < poly_data->step_data.tri_vlist.size(); i += 3){
				unsigned int vi1 = poly_data->step_data.tri_vlist[i];
				unsigned int vi2 = poly_data->step_data.tri_vlist[i + 1];
				unsigned int vi3 = poly_data->step_data.tri_vlist[i + 2];
				glVertex3f(poly_data->step_data.intersect_coord[vi1 * 3], poly_data->step_data.intersect_coord[vi1 * 3 + 1], poly_data->step_data.intersect_coord[vi1 * 3 + 2]);
				glVertex3f(poly_data->step_data.intersect_coord[vi2 * 3], poly_data->step_data.intersect_coord[vi2 * 3 + 1], poly_data->step_data.intersect_coord[vi2 * 3 + 2]);
				glVertex3f(poly_data->step_data.intersect_coord[vi3 * 3], poly_data->step_data.intersect_coord[vi3 * 3 + 1], poly_data->step_data.intersect_coord[vi3 * 3 + 2]);


			}
			glEnd();
		}
		
		
	}

	if (show_steps && use_seed_mode && poly_data->step_data.step_i >= 0){
		glBegin(GL_POINTS);
		glColor3f(0, 1, 1);
		deque<xyz> const * seed_queue = M->get_seed_queue();
		deque<xyz>::const_iterator iter = seed_queue->begin();
		while (iter != seed_queue->end()){
			glVertex3f(iter->x + M->get_grid_size() / 2.0, iter->y + M->get_grid_size() / 2.0, iter->z + M->get_grid_size() / 2.0);
			iter++;
		}
		glEnd();
	}
	glFlush();
	glutSwapBuffers();
}


/**
*	Function invoked when window system events are not being received
*/
void m_test_idle()
{
	/* Redraw the window */
	glutPostRedisplay();
}


/**
*	Function invoked when an event on regular keys occur
*/
void m_test_keyboard(unsigned char k, int x, int y)
{
	/* Show which key was pressed */
	//std::cout << "Pressed \"" << k << "\" ASCII: " << (int)k << std::endl;
	float dist = 10.0f;

	/* Close application if ESC is pressed */
	if (k == 27)
	{
		exit(0);
	}
	switch (k){
	case 'r':
		M->recalculate();
		break;
	
	case 'a':
		angle -= 0.05f;
		eyex = sin(angle) * dist;
		eyey = cos(angle) * dist;
		break;
	case 'd':
		angle += 0.05f;
		eyex = sin(angle) * dist;
		eyey = cos(angle) * dist;
		break;
	case 'm':
		M->step_by_step_mode(show_steps = !show_steps);
		break;
	case 'n':
		M->reset_step();
		break;
	case 'i':
		M->set_surface_constant(surface_constant += .1);
		M->recalculate();
		break;
	case 'u':
		M->set_surface_constant(surface_constant -= .1);
		M->recalculate();
		break;
	case 'o':
		M->set_surface_repeat_step_distance(1.5);
		M->repeating_surface_mode(true);
		M->recalculate();
		break;
	case 'p':
		M->repeating_surface_mode(false);
		M->recalculate();
		break;
	case 'l':
		M->load_poly_from_file();
		break;
	case 'k':
		M->save_poly_to_file();
		break;
	case 'g':
		M->set_constraint0("x", ">", -0.5);
		M->use_constraint0(using_constraint_0 = !using_constraint_0);
		M->recalculate();
		break;
	case 's':
		scaling_x *= 1.1;
		M->set_scaling_x(scaling_x);
		M->recalculate();
		break;
	case'w':
		scaling_x /= 1.1;
		M->set_scaling_x(scaling_x);
		M->recalculate();
		break;
	}
	//cout << angle << ":" << eyex << " " << eyey << endl;
}


/**
*	Set OpenGL initial state
*/
void m_test_init()
{
	/* Set clear color */
	glClearColor(1.0, 1.0, 1.0, 0.0);

	//glViewport(0, 0, m_test_windowWidth, m_test_windowHeight);
	//gluOrtho2D(-1.0, 1.0, 1.0, -1.0);
	
}



void test_marching(Marching* m)
{
	/* Initialize the GLUT window */
	cout << "r: refresh\na/d: turn\nm: turn on step mode (then press 'r' to step)\nn: reset step mode\n";
	cout << "i/u: change surface constant\no: turn on repeating surface\np: turn off repeating surface\n";
	cout << "l: load file \n k:save file\n";

	int argc = 0; char** argv = {};
	glutInit(&argc, argv);
	glutInitWindowSize(m_test_windowWidth, m_test_windowHeight);
	glutInitWindowPosition(700, 30);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	m_test_windowID = glutCreateWindow("Test Marching");

	M = m;
	
	M->step_by_step_mode(show_steps);
	M->set_surface_constant(surface_constant);
	//M->find_ambiguous_cases();
	/* Set OpenGL initial state */
	m_test_init();

	/* Callback functions */
	glutDisplayFunc(m_test_display);
	glutIdleFunc(m_test_idle);
	glutKeyboardFunc(m_test_keyboard);
	
	/* Start the main GLUT loop */
	/* NOTE: No code runs after this */
	glutMainLoop();
}

