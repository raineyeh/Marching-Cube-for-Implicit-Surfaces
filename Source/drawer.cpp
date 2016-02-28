#include "drawer.h"
#include "imgui_impl_glut.h"
#include <algorithm>
#include <memory>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "InitShader.h"
#include "vao.h"
using namespace std;

/* Window information */
float windowWidth = 1200;
float windowHeight = 800;
int windowID = -1;
/* Data information */
const Poly_Data* pData;
static const std::string vertex_shader("..\\..\\Source\\vs.glsl");
static const std::string fragment_shader("..\\..\\Source\\fs.glsl");
GLuint shader_program = -1;
GLuint vao = -1 ;
glm::mat4 P;
Drawer* pDrawer = nullptr;
bool hasInit = false;
void draw_gui()
{	
	ImGui_ImplGlut_NewFrame("Marching Cube");
	ImVec2 wsize(400.0f, windowHeight);
	ImGui::SetWindowFontScale(1.5);
	ImGui::SetWindowSize(wsize);
	ImVec2 wpos(windowWidth - 400.0f, 0);
	ImGui::SetWindowPos(wpos);
	static char buf[256] = "x^2 + y^2 = 0";
	static float grid = 0.02f;
	ImGui::InputText("Polynomial", buf, 256, 0);
	ImGui::SliderFloat("Grid size", &grid, 0.01f, 0.1f);
	
	if (ImGui::Button("Refresh") && pDrawer && pDrawer->Get_poly_data() &&
		!pData->tri_list.empty() && !pData->vertex_list.empty()){			
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);		
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pData->tri_list.size()*sizeof(GL_UNSIGNED_SHORT), &pData->tri_list[0], GL_DYNAMIC_DRAW); 
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, pData->vertex_list.size()*sizeof(float), &pData->vertex_list[0], GL_DYNAMIC_DRAW); 
		glBindBuffer(GL_ARRAY_BUFFER, 0);		
	}	
	ImGui::Render();
	hasInit = true;
}
void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glm::mat4 M = glm::mat4(1.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUseProgram(shader_program);
	int PVM_loc = glGetUniformLocation(shader_program, "PVM");
	if (PVM_loc != -1){
		glm::mat4 PVM = P*V*M;
		glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
	}
	int color_loc = glGetUniformLocation(shader_program, "ucolor");
	glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	if (color_loc != -1)
		glUniform4fv(color_loc, 1, glm::value_ptr(color));
		
	if (pData){
		glBindVertexArray(vao);	
		glDrawElements(GL_LINES, pData->vertex_list.size() , GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}		
	draw_gui();	
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
void reshape(int w, int h)
{
	glViewport(0, 0, w - 400, h );
	windowWidth = w;
	windowHeight = h;
	P = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f);
}

void reload_shader()
{
	GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());
	if (new_shader == -1) // loading failed
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
	}
	else
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		if (shader_program != -1)
		{
			glDeleteProgram(shader_program);
		}
		shader_program = new_shader;
	}
}

Drawer::Drawer(int* argc, char** argv){

	/* Initialize the GLUT window */
	glutInit(argc, argv);
	
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(30, 30);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	windowID = glutCreateWindow("Lalala");

	glewInit();
	ImGui_ImplGlut_Init();
	reload_shader();
	init_buffer();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(2.0);
	/* Callback functions */
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutKeyboardUpFunc(keyboard_up);
	glutSpecialUpFunc(special_up);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);	
}

bool Drawer::set_march(Marching* m){
	marching_obj = m;
	return true;
}

bool Drawer::Get_poly_data()
{
	if (marching_obj == nullptr) return false;
	bool ret = marching_obj->recalculate();
	pData = marching_obj->get_poly_data();
	return ret;
}

void Drawer::start(){

	/* Start the main GLUT loop */
	/* NOTE: No code runs after this */	
	pDrawer = this;
	glutMainLoop();
	int a = 0;
}