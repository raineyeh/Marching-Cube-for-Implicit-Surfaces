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
float windowWidth = 1050;
float windowHeight = 700;
float barwidth = 350;
int windowID = -1;
/* Data information */
const Poly_Data* pData;
static const std::string vertex_shader("..\\..\\Source\\vs.glsl");
static const std::string fragment_shader("..\\..\\Source\\fs.glsl");
static char buf[256] = "x^2+y^2-0.5";
static float fGrid = 0.2f;
static bool bMode = false;
GLuint shader_program = -1;
GLuint vao = -1 ;
glm::mat4 P;
Drawer* pDrawer = nullptr;
bool hasInit = false;

void drawbuffer(int ni,int nv,void* pi,void* pv){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni, pi, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, nv, pv, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void draw_gui()
{	
	ImGui_ImplGlut_NewFrame("Marching Cube");
	ImVec2 wsize(barwidth, windowHeight);
	ImGui::SetWindowFontScale(1.5);
	ImGui::SetWindowSize(wsize);
	ImVec2 wpos(windowWidth - barwidth, 0);
	ImGui::SetWindowPos(wpos);
	
	ImGui::InputText("Polynomial", buf, 256, 0);
	if (ImGui::SliderFloat("Grid size", &fGrid, 0.05f, 0.5f)){
		if (pDrawer)
			pDrawer->SetGridSize(fGrid);
	}
	if (ImGui::Button("Refresh") && pDrawer){			
		pDrawer->SetEquation(string(buf));
		pDrawer->SetGridSize(fGrid);
		pDrawer->Get_poly_data();

		if (pData && !pData->tri_list.empty() && !pData->vertex_list.empty())
			drawbuffer(pData->tri_list.size()*sizeof(unsigned int), pData->vertex_list.size()*sizeof(float),
				(void*)&pData->tri_list[0], (void*)&pData->vertex_list[0]);		
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear")){
		drawbuffer(0, 0, nullptr, nullptr);
	}
	if (ImGui::Checkbox("Step Mode",&bMode)){
		if (pDrawer)
			pDrawer->SetStepMode(bMode);
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
	switch (key)
	{
	case GLUT_KEY_RIGHT:	
		if (pDrawer)
			pDrawer->Get_poly_data();
		if (pData && !pData->tri_list.empty() && !pData->vertex_list.empty())
			drawbuffer(pData->tri_list.size()*sizeof(unsigned int), pData->vertex_list.size()*sizeof(float),
			(void*)&pData->tri_list[0], (void*)&pData->vertex_list[0]);
		break;
	}	
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
	glViewport(0, 0, w - barwidth, h);
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
	m_pEvaluator = nullptr;
	m_pMmarching = nullptr;
}

bool Drawer::set_march(Marching* m){
	m_pMmarching = m;
	return true;
}

bool Drawer::Get_poly_data()
{
	if (m_pMmarching == nullptr) 
		return false;

	m_pMmarching->recalculate();
	if (pData == nullptr) 
		pData = m_pMmarching->get_poly_data();
	return pData != nullptr;
}

void Drawer::start(){

	/* Start the main GLUT loop */
	/* NOTE: No code runs after this */	
	pDrawer = this;
	glutMainLoop();
	int a = 0;
}

void Drawer::SetGridSize(float fGrid)
{
	if (m_pMmarching)
		m_pMmarching->set_grid_step_size(fGrid);
}

void Drawer::SetStepMode(bool mode)
{
	if (m_pMmarching)
		m_pMmarching->step_by_step_mode(mode);
}

void Drawer::SetEquation(string s)
{
	if (m_pEvaluator)
		m_pEvaluator->set_equation(s);	
}

void Drawer::set_evaluator(Evaluator* e)
{
	m_pEvaluator = e;
}
