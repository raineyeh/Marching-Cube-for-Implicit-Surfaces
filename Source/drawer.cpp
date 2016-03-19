#define GLM_FORCE_RADIANS
#include "drawer.h"
#include "imgui/imgui_impl_glut.h" 
#include <algorithm>
#include <memory> 
#include <process.h> 
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glslprogram.h"
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
static float fGrid = 0.25f;
static float fInterval = 0.2f;
static bool bStepMode = false;
static bool bTranslucent = false;
bool bHasInit = false;
bool bPressed = false;
bool bMovie = false;
int last_mx = 0, last_my = 0, cur_mx = 0, cur_my = 0;
GLuint vao[3], vbo[3], ibo[3];//0 for model, 1 for cube, 2 for intersect
glm::mat4 M,V,P;
vector<float> vIntersectVertex, vIntersectIndex;
// objects 
MyFile myfile;
Drawer* pDrawer = nullptr;
GLSLProgram program;
//cube idx
unsigned int idxCube[36] = { 0, 2, 1, 2, 0, 3, 0, 5, 4, 5, 0, 1, 1, 6, 5, 6, 1, 2, 2, 7, 6, 7, 2, 3,3, 4, 7, 4, 3, 0, 4, 5, 6, 6, 7, 4 };
unsigned int idxEdge[30] = { 0, 1, 2, 3, 0, 1, 5, 6, 2, 1, 5, 4, 7, 6, 5, 4, 0, 3, 7, 4, 4, 5, 1, 0, 4, 7, 3, 2, 6, 7 };

void BufferData(GLuint ibo, GLuint ni, void* pi, GLuint vbo, GLuint nv, void* pv){	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, nv, pv, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni, pi, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void SetStepData(){
	if (pData == nullptr) return;

	//model
	if (!pData->tri_list.empty() && !pData->vertex_list.empty()){
		BufferData(ibo[0], pData->tri_list.size()*sizeof(unsigned int), (void*)&pData->tri_list[0],
			vbo[0], pData->vertex_list.size()*sizeof(float), (void*)&pData->vertex_list[0]);
	}

	//cube????
	BufferData(0, 0, 0, vbo[1], pData->step_data.corner_coords.size()*sizeof(float), (void*)&pData->step_data.corner_coords[0]);

	//intersect	
	int num = pData->step_data.intersect_coord.size();
	if (num > 0){
		BufferData(0, 0, 0, vbo[2], pData->step_data.intersect_coord.size()*sizeof(float), (void*)&pData->step_data.intersect_coord[0]);
	}
}
void Movie(void*)
{		
	if (pDrawer == nullptr) return;	
	pDrawer->ResetStep();
	pDrawer->SetStepMode(true);
	bStepMode = true;
	pDrawer->SetEquation(string(buf));
	pDrawer->SetGridSize(fGrid);
	pDrawer->Recalculate();
	pDrawer->GetPolyData();
	for (int i = 0; bMovie && pData && i != pData->step_data.step_i;){
		pDrawer->Recalculate();
		SetStepData();	
		Sleep(50);		
	}	
	pDrawer->SetStepMode(false);
	pDrawer->ResetStep();
	bStepMode = false;
	bMovie = false;	
	SetEvent(pDrawer->m_hEvent);
	_endthread();	
}
void WaitForEnd(){	
	MSG msg = { 0 };
	while (pDrawer){
		DWORD result = MsgWaitForMultipleObjects(1, &pDrawer->m_hEvent, FALSE, INFINITE, QS_ALLINPUT);
		if (result == (WAIT_OBJECT_0)){
			for (int i = 0; i < 3; i++){
				BufferData(ibo[i], 0, 0, vbo[i], 0, 0);
				return;
			}
		}
		else{
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			DispatchMessage(&msg);
		}
	}
}

void DrawGUI()
{	
	ImGui_ImplGlut_NewFrame("Marching Cube");
	ImVec2 wsize(barwidth, windowHeight);
	ImGui::SetWindowFontScale(1.5);
	ImGui::SetWindowSize(wsize);
	ImVec2 wpos(windowWidth - barwidth, 0);
	ImGui::SetWindowPos(wpos);
	if (!bMovie)
		ImGui::InputText("Polynomial", buf, 256, 0);
	if (!bMovie && ImGui::SliderFloat("Grid size", &fGrid, 0.1f, 0.5f)){
		if (pDrawer)
			pDrawer->SetGridSize(fGrid);
	}
	if (!bMovie && ImGui::SliderFloat("Level set", &fInterval, 0.1f, 0.5f)){
		if (pDrawer)
			pDrawer->SetGridSize(fGrid);
	}
	if (!bMovie && ImGui::Button("Refresh") && pDrawer){
		pDrawer->SetEquation(string(buf));
		pDrawer->SetGridSize(fGrid);
		pDrawer->Recalculate();
		pDrawer->GetPolyData();		
		if (pData && !pData->tri_list.empty() && !pData->vertex_list.empty())
			BufferData(ibo[0], pData->tri_list.size()*sizeof(unsigned int), (void*)&pData->tri_list[0],
					   vbo[0], pData->vertex_list.size()*sizeof(float),(void*)&pData->vertex_list[0]);		
	}
	ImGui::SameLine();
	if (ImGui::Button(bMovie?"Stop":"Movie")){
		if (bMovie){
			bMovie = false;		
			WaitForEnd();			
		}
		else{
			_beginthread(Movie, 0, 0);
			bMovie = true;
		}		
	}
	ImGui::SameLine();
	if (!bMovie && ImGui::Button("Reset") ){
		if (pDrawer)
			pDrawer->ResetStep();
		for (int i = 0; i < 3;i++){
			BufferData(ibo[i], 0, 0, vbo[i], 0, 0);
		}		
	}	
	if (!bMovie && ImGui::Button("Save mesh")){
		myfile.Save(pData);
	}
	ImGui::SameLine();
	if (!bMovie && ImGui::Button("Load mesh")){
		myfile.Open();
	}
	if (!bMovie && ImGui::Checkbox("Step Mode", &bStepMode)){
		if (pDrawer)
			pDrawer->SetStepMode(bStepMode);
	}ImGui::SameLine();
	if (ImGui::Checkbox("Translucent", &bTranslucent)){
		if (bTranslucent)
			glDisable(GL_DEPTH_TEST); 
		else
			glEnable(GL_DEPTH_TEST);	
		program.setUniform("uTranslucent", bTranslucent);
	}
	char ch[15] = {0};
	if (pData && bStepMode && pData->step_data.step_i>=0)
		sprintf_s(ch, "Steps:%d", pData->step_data.step_i);
	ImGui::Text(ch);
	ImVec4 col = ImVec4(1.0, 0.0, 0.0, 1.0);
	ImGui::TextColored(col,"Red points for positive");
	col = ImVec4(0.0, 1.0, 0.0, 1.0);
	ImGui::TextColored(col, "Green points for negative");
	col = ImVec4(0.0, 0.0, 1.0, 1.0);
	ImGui::TextColored(col, "Blue points for intersect");
	col = ImVec4(1.0, 1.0, 1.0, 1.0);
	ImGui::TextColored(col, "Right arrow for next step");
	ImGui::Render();
	bHasInit = true;	
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
	program.setUniform("M", M);
	program.setUniform("V", V);
	program.setUniform("P", P);
}
void DrawCube(){
	if (!bStepMode) return;	
	if (pData == nullptr) return;
	if (pData->step_data.corner_coords.empty()) return;		
	
	SetStepData();

	//cube	
	BufferData(ibo[1], sizeof(idxCube), idxCube, 0, 0, 0);//修改数据后要bind
	glm::vec4 color = glm::vec4(0.0f, 0.0f, 1.0f, 0.2f);	
	program.setUniform("ucolor", color);
	glBindVertexArray(vao[1]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	
	//edge
	BufferData(ibo[1], sizeof(idxEdge), idxEdge, 0, 0, 0);
	color = glm::vec4(0.5f, 0.5f, 0.5f, 0.8f);
	program.setUniform("ucolor", color);
	glBindVertexArray(vao[1]);
	glDrawElements(GL_LINE_STRIP, 30, GL_UNSIGNED_INT, 0);
	
	//corner	
	int n = 0;
	for (auto p : pData->step_data.corner_values){
		color = p < 0 ? glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) : glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		program.setUniform("ucolor", color);
		glDrawArrays(GL_POINTS, n++, 1);		
	}
		
	//intersect	
	glBindVertexArray(vao[2]);
	color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	program.setUniform("ucolor",color);
	glDrawArrays(GL_POINTS, 0, pData->step_data.intersect_coord.size()/3);
		
	glBindVertexArray(0);	
}
void DrawModel(){		
	if (pData == nullptr) return;

	glBindVertexArray(vao[0]);
	glm::vec4 color = glm::vec4(0.3f, 0.1f, 0.1f, bTranslucent ? 0.5f : 1.0f);
	program.setUniform("ucolor", color);
	glDrawElements(GL_TRIANGLES, pData->vertex_list.size(), GL_UNSIGNED_INT, 0);
	color = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
	program.setUniform("ucolor", color);
	glDrawElements(GL_LINES, pData->vertex_list.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);	
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	Arcball();
	DrawCube();
	DrawModel();
	DrawGUI();
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y){
	ImGui_ImplGlut_KeyCallback(key);	
}

void keyboard_up(unsigned char key, int x, int y){
	ImGui_ImplGlut_KeyUpCallback(key);
}

void special_up(int key, int x, int y){
	ImGui_ImplGlut_SpecialUpCallback(key);
}

void passive(int x, int y){
	ImGui_ImplGlut_PassiveMouseMotionCallback(x, y);
}
void special(int key, int x, int y){
	ImGui_ImplGlut_SpecialCallback(key);
	if (bStepMode == false) return;
	switch (key)
	{
	case GLUT_KEY_RIGHT:
		if (pDrawer) {
			pDrawer->Recalculate();
			pDrawer->GetPolyData();
		}			
		if (pData == nullptr) break;
		SetStepData();
		break;
	}
}

void motion(int x, int y){
	ImGui_ImplGlut_MouseMotionCallback(x, y);
	if (bPressed) {  // if left button is pressed
		cur_mx = x;
		cur_my = y;
	}
}

void mouse(int button, int state, int x, int y)
{
	ImGui_ImplGlut_MouseButtonCallback(button, state);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		bPressed = true;
		last_mx = cur_mx = x;
		last_my = cur_my = y;
	}
	else {
		bPressed = false;
	}
}

void idle(){
	glutPostRedisplay();
}
void reshape(int w, int h){
	glViewport(0, 0, w - barwidth, h);
	windowWidth = w;
	windowHeight = h;
}

void CompileAndLinkShader(){
	try {
		program.compileShader(vertex_shader.c_str(), GLSLShader::VERTEX);
		program.compileShader(fragment_shader.c_str(), GLSLShader::FRAGMENT);
		program.link();
		program.use();
	}
	catch (GLSLProgramException & e) {
		cerr << e.what() << endl;
		system("pause");
	//	exit(EXIT_FAILURE);
	}
}
void InitMatrix(){
	M = glm::mat4(1.0f);
	V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	P = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);	
}
void InitBuffer(){	
	for (int i = 0; i < 3;i++){
		CreateBuffer(program.getHandle(),vao[i],vbo[i],ibo[i]);
	}	
}
Drawer::Drawer(int* argc, char** argv){

	/* Initialize the GLUT window */
	glutInit(argc, argv);
	
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(30, 30);
	glutSetOption(GLUT_MULTISAMPLE, 4); //Set number of samples per pixel
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	windowID = glutCreateWindow("Marching Cube");

	glewInit();
	ImGui_ImplGlut_Init();
	CompileAndLinkShader();
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPointSize(8.0);	
//	glFrontFace()//设定正反面winding order
//	glCullFace()//剔除面
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
	InitBuffer();
	m_pEvaluator = nullptr;
	m_pMmarching = nullptr;		
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, "Event");	
}

bool Drawer::set_march(Marching* m){
	m_pMmarching = m;
	return true;
}

bool Drawer::GetPolyData()
{	
	if (pData == nullptr) 
		pData = m_pMmarching->get_poly_data();
	return pData != nullptr;
}

void Drawer::Recalculate()
{
	if (m_pMmarching)	
		m_pMmarching->recalculate();
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

void Drawer::SetEquation(string s){
	if (m_pEvaluator)
		m_pEvaluator->set_equation(s);	
}

void Drawer::ResetStep(){
	if (m_pMmarching)
		m_pMmarching->reset_step();
}

void Drawer::set_evaluator(Evaluator* e){
	m_pEvaluator = e;
}
