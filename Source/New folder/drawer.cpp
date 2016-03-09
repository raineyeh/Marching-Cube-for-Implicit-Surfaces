#define GLM_FORCE_RADIANS
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
#include "MyFile.h"
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
static bool bStepMode = false;
GLuint shader_program = -1;
GLuint vao = -1 ;
Drawer* pDrawer = nullptr;
bool hasInit = false;
bool pressed = false;
int last_mx = 0, last_my = 0, cur_mx = 0, cur_my = 0;
glm::mat4 V;
glm::mat4 M;
glm::mat4 P;
MyFile myfile;
void BufferData(int ni,int nv,void* pi,void* pv){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni, pi, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, nv, pv, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void DrawGUI()
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
	//	pDrawer->SetEquation(string(buf));
		pDrawer->SetGridSize(fGrid);
		pDrawer->Get_poly_data();

		if (pData && !pData->tri_list.empty() && !pData->vertex_list.empty())
			BufferData(pData->tri_list.size()*sizeof(unsigned int), pData->vertex_list.size()*sizeof(float),
				(void*)&pData->tri_list[0], (void*)&pData->vertex_list[0]);		
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear")){
		BufferData(0, 0, nullptr, nullptr);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load File")){
		myfile.Open();
	}
	if (ImGui::Checkbox("Step Mode",&bStepMode)){
		if (pDrawer)
			pDrawer->SetStepMode(bStepMode);
	}
	
	char ch[10] = {0};
	if (pData)
		sprintf_s(ch, "Steps:%d", pData->step_data.step_i);
	ImGui::LabelText("", ch);	
	ImGui::Render();
	hasInit = true;	
}
glm::vec3 GetArcballVector(int x, int y) {
	glm::vec3 P = glm::vec3(1.0*x / windowWidth * 2 - 1.0,
		1.0*y / windowHeight * 2 - 1.0,
		0);//屏幕坐标系变为[-1,1]
	P.y = -P.y;
	float OP_squared = P.x * P.x + P.y * P.y;
	if (OP_squared <= 1 * 1)
		P.z = sqrt(1 * 1 - OP_squared);  // Pythagore
	else
		P = glm::normalize(P);  // nearest point
	return P;
}

void Arcball() {	
	// Handle 
	if (cur_mx != last_mx || cur_my != last_my) {
		glm::vec3 va = GetArcballVector(last_mx, last_my);
		glm::vec3 vb = GetArcballVector(cur_mx, cur_my);
		float angle = acos(min(1.0f, glm::dot(va, vb)));
		glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
		glm::mat3 camera2object = glm::inverse(glm::mat3(V) * glm::mat3(M));
		glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
		M = glm::rotate(M, angle, axis_in_object_coord);
		last_mx = cur_mx;
		last_my = cur_my;
	}

	// Model
	// call in main_object.draw() - main_object.M
	int PVM_loc = glGetUniformLocation(shader_program, "PVM");
	if (PVM_loc != -1){
		glm::mat4 PVM = P*V*M;
		glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
	}
}
void DrawCube(){
	if (!bStepMode) return;
	
	
}
void DrawModel(){
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shader_program);

	int color_loc = glGetUniformLocation(shader_program, "ucolor");
	glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	if (color_loc != -1)
		glUniform4fv(color_loc, 1, glm::value_ptr(color));

	if (pData){
		glBindVertexArray(vao);
		glDrawElements(GL_LINES, pData->vertex_list.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}	
}
void display()
{
	Arcball();
	DrawModel();
	DrawGUI();
	glutSwapBuffers();
}

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
			BufferData(pData->tri_list.size()*sizeof(unsigned int), pData->vertex_list.size()*sizeof(float),
			(void*)&pData->tri_list[0], (void*)&pData->vertex_list[0]);
		break;
	}	
}

void motion(int x, int y)
{
	ImGui_ImplGlut_MouseMotionCallback(x, y);
	if (pressed) {  // if left button is pressed
		cur_mx = x;
		cur_my = y;
	}
}

void mouse(int button, int state, int x, int y)
{
	ImGui_ImplGlut_MouseButtonCallback(button, state);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		pressed = true;
		last_mx = cur_mx = x;
		last_my = cur_my = y;
	}
	else {
		pressed = false;
	}
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
void InitMatrix(){
	M = glm::mat4(1.0f);
	V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	P = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);
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
	InitMatrix();
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
