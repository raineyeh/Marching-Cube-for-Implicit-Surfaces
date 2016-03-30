#define GLM_FORCE_RADIANS
#include "drawer.h"
#include "imgui_impl_glut.h" 
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
#include "ShaderLib.h"
#include "tinycthread.h"
#include "vao.h"
#include "MyFile.h"
using namespace std;

/* Window information */
int nBase = 700;
int nPolynomialHeight = 110;
int nWindowHeight = nBase + nPolynomialHeight;
int nBarWidth = 380;
int nWindowWidth = nBase + nBarWidth;

int windowID = -1;
float nColor = 48;
/* Data information */
const Poly_Data* pData;
static const std::string vertex_shader("..\\..\\Source\\vs.glsl");
static const std::string fragment_shader("..\\..\\Source\\fs.glsl");
static char szInput[256] = "x^2+y";//x^2+y^2-0.5   (y-0.1)^2-(z*z+2*x)^2+0.1
static char szScaler[256] = "1.0";
static char szSurfaceConstant[256] = "0.0";
static float fGrid = 0.25f;
static float fSurfaceConstant = 0.2f;
static float fStepDistance = 0.5f;
static float fPercent;
static float fSeeding[3];
static bool bStepMode,bSeedingMode, bTranslucent, bInvertFace, bRepeatingMode;
static ImVec4 colBackground = ImColor(1.0f, 1.0f,1.0f, 1.0f);
static ImVec4 colModelFrontFace = ImColor(1.0f, 0.3f, 0.3f, 1.0f);
static ImVec4 colModelBackFace = ImColor(0.2f, 0.2f, 0.2f, 0.5f);
static ImVec4 colCubeSurface = ImColor(0.0f, 0.0f, 1.0f, 0.5f);
static ImVec4 colCubeEdge = ImColor(1.0f, 0.0f, 1.0f, 1.0f);
static ImVec4 colModelEdge = ImColor(0.0f, 0.0f,0.0f, 1.0f);
static ImVec4 colIntersectPoint = ImColor(0.0f, 0.0f, 1.0f, 1.0f);
static ImVec4 colIntersectSurface = ImColor(0.0f, 0.0f, 1.0f, 1.0f);
static ImVec4 colInCornerPoint = ImColor(0.0f, 1.0f, 0.0f, 1.0f);
static ImVec4 colOutCornerPoint = ImColor(1.0f, 0.0f, 0.0f, 1.0f);
bool bLeftPressed, bRightPressed, bHasInit, bMovie, bPause, bCubeStep;
int nLastX, nLastY, nCurX, nCurY;
int nCubeStep;
GLuint vao[4], vbo[4], ibo[4];//0 for model, 1 for cube, 2 for intersect, 3 for intersect polugon
glm::mat4 M,V,P;
vector<float> vIntersectVertex, vIntersectIndex;
// objects 
MyFile myfile;
Drawer* pDrawer = nullptr;
ShaderLib ModelShader;
thrd_t MovieThread;
//cube idx
unsigned int idxCube[36] = { 1, 0, 2, 2, 0, 3, 0, 5, 4, 5, 0, 1, 1, 6, 5, 6, 1, 2, 2, 7, 6, 7, 2, 3,3, 0, 7, 0, 4, 7, 4, 5, 6, 6, 7, 4 };
unsigned int idxEdge[30] = { 0, 1, 2, 3, 0, 1, 5, 6, 2, 1, 5, 4, 7, 6, 5, 4, 0, 3, 7, 4, 4, 5, 1, 0, 4, 7, 3, 2, 6, 7 };
//states
enum CubeStates{
	GRID = 0,
	CORNERPOINT,
	INTERSECTIONS,
	POLYGON
};
void BufferData(GLuint ibo, GLuint ni, void* pi, GLuint vbo, GLuint nv, void* pv){	
	glNamedBufferData(vbo, nv, pv, GL_DYNAMIC_DRAW);
	glNamedBufferData(ibo, ni, pi, GL_DYNAMIC_DRAW);
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

static int Movie(void*)
{		
	if (pDrawer == nullptr) return false;
	if (pData == nullptr) return false;
	float fTotalStep = (float)pData->step_data.step_i;
	
	for (int i = 0; bMovie && i != pData->step_data.step_i;){
		if (bPause) continue;
		fPercent = pData->step_data.step_i / fTotalStep;
		fPercent = 1.0f - fPercent;
		pDrawer->Recalculate();	
		if (pData->step_data.intersect_coord.size()>0)
		for (int nStep = 0; nStep < 4; nStep++)
		{
			nCubeStep++;
			Sleep(200);
		}
		nCubeStep = -1;
		Sleep(50);		
	}	
	pDrawer->SetStepMode(false);
	pDrawer->ResetStep();
	bStepMode = false;
	bMovie = false;		
	fPercent = 0;
	_endthread();	
	return true;
}
void DrawGUI()
{	
	ImGui_ImplGlut_NewFrame("Marching Cube",0.96f);
	ImVec2 wsize(nBarWidth, nWindowHeight - nPolynomialHeight);
	ImGui::SetWindowFontScale(1.5);
	ImGui::SetWindowSize(wsize);
	ImVec2 wpos(nWindowWidth - nBarWidth, 0);
	ImGui::SetWindowPos(wpos);
	//if (!bMovie){
	//	ImGui::InputText("1", szInput, 256, 0);
	//	ImGui::Text("			Polynomial");		
	//}		
	if (!bMovie && ImGui::SliderFloat("2", &fGrid, 0.1f, 0.5f, "Grid size: %.2f")){
		if (pDrawer) {
			pDrawer->SetGridSize(fGrid);
			pDrawer->ResetStep();
		}			
	}
/*	if (!bMovie && ImGui::SliderFloat("3", &fSurfaceConstant, 0.1f, 1.0f, "Surface constant: %.2f")){
		if (pDrawer){
			pDrawer->SetSurfaceConstant(fSurfaceConstant);
			pDrawer->ResetStep();
		}			
	}
*/
	if (!bMovie && ImGui::Button("Refresh") && pDrawer){
		pDrawer->SetEquation(string(szInput));
		pDrawer->SetGridSize(fGrid);
		pDrawer->Recalculate();
		pDrawer->GetPolyData();		
		if (pData && !pData->tri_list.empty() && !pData->vertex_list.empty())
			BufferData(ibo[0], pData->tri_list.size()*sizeof(unsigned int), (void*)&pData->tri_list[0],
				   vbo[0], pData->vertex_list.size()*sizeof(float),(void*)&pData->vertex_list[0]);	

	}
	ImGui::SameLine();
	if (ImGui::Button(bMovie ? "Stop" : "Movie")){
		if (bMovie){
			bMovie = false;
			bPause = false;
			thrd_join(MovieThread, NULL);
			MovieThread = 0;
			for (int i = 0; i < 3; i++)
				BufferData(ibo[i], 0, 0, vbo[i], 0, 0);									
		}
		else{	
			bStepMode = true;
			bMovie = true;
			bPause = false;
			if (pDrawer == nullptr) return;
			pDrawer->ResetStep();
			pDrawer->SetStepMode(true);			
			pDrawer->SetEquation(string(szInput));
			pDrawer->SetGridSize(fGrid);
			pDrawer->Recalculate();
			pDrawer->GetPolyData();
			thrd_create(&MovieThread, Movie, NULL);				
		}		
	}
	if (bMovie){
		ImGui::SameLine();
		if (ImGui::Button(bPause?"Resume":"Pause")){
			if (bPause)
				bPause = false;
			else
				bPause = true;			
		}
	}	
	ImGui::SameLine();
	if (!bMovie && ImGui::Button("Reset") ){
		if (pDrawer)
			pDrawer->ResetStep();
		for (int i = 0; i < 3;i++)
			BufferData(ibo[i], 0, 0, vbo[i], 0, 0);		
	}	
	if (ImGui::Button("Save mesh")){
		if (pData == nullptr || pData && pData->vertex_list.size() == 0)
			ImGui::OpenPopup("Save error");	
		else{
			if (pDrawer)pDrawer->SaveFile();			
		}		
	}
	if (ImGui::BeginPopupModal("Save error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("No data to save.\n\n");
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup(); 
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Load mesh")){
		if (pDrawer) {
			pDrawer->LoadFile();
			pDrawer->ResetStep();
			pDrawer->GetPolyData();
		}
		if (pData && !pData->tri_list.empty() && !pData->vertex_list.empty())
			BufferData(ibo[0], pData->tri_list.size()*sizeof(unsigned int), (void*)&pData->tri_list[0],
			vbo[0], pData->vertex_list.size()*sizeof(float), (void*)&pData->vertex_list[0]);
		else ImGui::OpenPopup("Load error");
	}
	if (ImGui::BeginPopupModal("Load error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("No data to load.\n\n");
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (!bMovie && ImGui::Checkbox("Step mode", &bStepMode)){
		pDrawer->GetPolyData();
		
		
		for (int i = 0; i < 3; i++)
			BufferData(ibo[i], 0, 0, vbo[i], 0, 0);
		if (pDrawer){
			pDrawer->ResetStep();
			pDrawer->SetStepMode(bStepMode);
		}			
	}
	if (!bMovie && ImGui::Checkbox("Seeding mode", &bSeedingMode)){				
		if (pDrawer) pDrawer->SetSeedMode(bSeedingMode);
	}	
	if (!bMovie && ImGui::InputFloat3("5", fSeeding))
	if (pDrawer) pDrawer->SetSeed(fSeeding);
	ImGui::Text("		  Seeding point");
	
	if (!bMovie && ImGui::Checkbox("Repeating surface mode", &bRepeatingMode)){
		if (pDrawer){
			pDrawer->SetRepeatingSurfaceMode(bRepeatingMode);
			pDrawer->SetSurfaceRepeatStepDistance(fStepDistance);
			pDrawer->ResetStep();
		}		
	}
	if (!bMovie && ImGui::SliderFloat("4", &fStepDistance, 0.1f, 0.9f, "Surface repeat step distance:%.2f")){
		if (pDrawer){
			pDrawer->SetSurfaceRepeatStepDistance(fStepDistance);
			pDrawer->ResetStep();
		}
	}
	if (ImGui::Checkbox("Translucent", &bTranslucent)){
		if (bTranslucent){
			glDisable(GL_DEPTH_TEST); 				
		}
			
		else{
			glEnable(GL_DEPTH_TEST);						
		}			
		ModelShader.setUniform("uTranslucent", bTranslucent);
	}	
	if (bMovie){
		ImVec2 psize(nBarWidth, 30);
		ImGui::ProgressBar(fPercent, psize);
	}		  
	if (!bSeedingMode){
		char ch[20] = { 0 };
		if (pData && bStepMode && pData->step_data.step_i >= 0)
			sprintf_s(ch, "RemainSteps:%d", pData->step_data.step_i);
		ImGui::Text(ch);
	}		

	if (bRightPressed)
		ImGui::OpenPopup("menu");
	if (ImGui::BeginPopup("menu"))
	{
		if (ImGui::ColorPlate(&colBackground, "Background color")){
			glClearColor(colBackground.x, colBackground.y, colBackground.z, 1.0f);
		}
		ImGui::ColorPlate(&colModelFrontFace, "Surface front color");
		ImGui::ColorPlate(&colModelBackFace, "Surface back color");
		ImGui::ColorPlate(&colCubeEdge, "Cube edge color");
		ImGui::ColorPlate(&colModelEdge, "Surface edge color");
		ImGui::ColorPlate(&colInCornerPoint, "Inside point color");
		ImGui::ColorPlate(&colOutCornerPoint, "Outside point color");
		ImGui::ColorPlate(&colIntersectPoint, "Intersect point color");
		ImGui::ColorPlate(&colIntersectSurface, "Intersect surface color");
		ImGui::EndPopup();
	}

	//P
	if (!bMovie){	
	ImGui::Begin("ImGui Demo", 0.85f, false, ImVec2(nWindowHeight, 0), -1.0f, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	ImVec2 psize(nWindowWidth, nPolynomialHeight);	
	ImGui::SetWindowSize(psize);
	ImVec2 ppos(0, nWindowHeight-nPolynomialHeight);
	ImGui::SetWindowPos(ppos);
	ImGui::Text("P(sx,sy,sz)=");
	ImGui::SetWindowFontScale(4.0);
	ImGui::InputText("=", szInput, 256, 0);ImGui::SameLine();	
	ImGui::InputText("s", szSurfaceConstant, 256, 0);
	ImGui::SetWindowFontScale(1.0);
	ImGui::InputText("Scaler", szScaler, 256, 0);
	ImGui::End();
	}

	static bool show = false;
	// 	ImGui::ShowTestWindow(&show);

	ImGui::Render();
	bHasInit = true;	
}
glm::vec3 GetArcballVector(int x, int y) {
	glm::vec3 P = glm::vec3(1.0*x / nWindowWidth * 2 - 1.0,
		1.0*y / nWindowHeight * 2 - 1.0,
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
	if (nCurX != nLastX || nCurY != nLastY) {
		glm::vec3 va = GetArcballVector(nLastX, nLastY);
		glm::vec3 vb = GetArcballVector(nCurX, nCurY);
		float angle = acos(min(1.0f, glm::dot(va, vb)));
		glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
		glm::mat3 camera2object = glm::inverse(glm::mat3(V) * glm::mat3(M));
		glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
		M = glm::rotate(M, angle, axis_in_object_coord);
		nLastX = nCurX;
		nLastY = nCurY;
	}

	// Model
	// call in main_object.draw() - main_object.M
 	ModelShader.setUniform("M", M);
	ModelShader.setUniform("V", V); 
 	ModelShader.setUniform("P", P);
}
void DrawCube(){
	if (!bStepMode) return;	
	if (pData == nullptr) return;
	if (pData->step_data.corner_coords.empty()) return;		
	
	SetStepData();	

	//cube	
	glDepthMask(false);	
	BufferData(ibo[1], sizeof(idxCube), idxCube, 0, 0, 0);//修改数据后要bind
	ModelShader.setUniform("uFrontColor", glm::vec4(colCubeSurface.x, colCubeSurface.y, colCubeSurface.z, 0.5f));
	glBindVertexArray(vao[1]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glDepthMask(true);
	
	//edge	
	BufferData(ibo[1], sizeof(idxEdge), idxEdge, 0, 0, 0);
	ModelShader.setUniform("uFrontColor", glm::vec4(colCubeEdge.x, colCubeEdge.y, colCubeEdge.z, 1.0f));
	glBindVertexArray(vao[1]);
	glDrawElements(GL_LINE_STRIP, 30, GL_UNSIGNED_INT, 0);
		
	//corner	
	glDepthMask(false);
	if (nCubeStep > 0){
		int n = 0;
		for (auto p : pData->step_data.corner_values){
			glm::vec4 color = p < 0 ? glm::vec4(colInCornerPoint.x, colInCornerPoint.y, colInCornerPoint.z, 1.0f)
				: glm::vec4(colOutCornerPoint.x, colOutCornerPoint.y, colOutCornerPoint.z, 1.0f);
			ModelShader.setUniform("uFrontColor", color);
			glDrawArrays(GL_POINTS, n++, 1);
		}
	}
	//intersect	
	if (nCubeStep > 1){
		glBindVertexArray(vao[2]);
		ModelShader.setUniform("uFrontColor", glm::vec4(colIntersectPoint.x, colIntersectPoint.y, colIntersectPoint.z, 1.0f));
		glDrawArrays(GL_POINTS, 0, pData->step_data.intersect_coord.size() / 3);
	}
	glDepthMask(true);
	

	//polygon
	if (nCubeStep > 2){
		ModelShader.setUniform("uFrontColor", glm::vec4(colIntersectSurface.x, colIntersectSurface.y, colIntersectSurface.z, 1.0f));
		ModelShader.setUniform("uBackColor", glm::vec4(colIntersectSurface.x, colIntersectSurface.y, colIntersectSurface.z, colModelBackFace.w));
		glDrawArrays(GL_TRIANGLE_STRIP, 0, pData->step_data.intersect_coord.size() / 3);
	}
	glBindVertexArray(0);	
}

void DrawModel(){		
	if (pData == nullptr) return;
	
	glBindVertexArray(vao[0]); 	
	ModelShader.setUniform("uFrontColor", glm::vec4(colModelFrontFace.x, colModelFrontFace.y, colModelFrontFace.z, bTranslucent ? 0.5f : 1.0f));
	ModelShader.setUniform("uBackColor", glm::vec4(colModelBackFace.x, colModelBackFace.y, colModelBackFace.z, colModelBackFace.w));
	glDrawElements(GL_TRIANGLES, (pData->vertex_list.size()-2)*3, GL_UNSIGNED_INT, 0);

	glLineWidth(3);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	
	ModelShader.setUniform("uFrontColor", glm::vec4(colModelEdge.x, colModelEdge.y, colModelEdge.z, 1.0f));
	glDrawElements(GL_TRIANGLES, (pData->vertex_list.size() - 2) * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	Arcball();
	DrawModel();
	DrawCube();	
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
		if (pData == nullptr) break;
		if (pDrawer) {
			pDrawer->SetEquation(string(szInput));

			if (bCubeStep){
				nCubeStep++;
				if (nCubeStep == 4) {
					nCubeStep = 0;
					pDrawer->Recalculate();
				}
				else break;
			}			
			else 
				pDrawer->Recalculate();

			bCubeStep = pData->step_data.intersect_coord.size() > 0 ? true : false;					
		}		
		SetStepData();
		break;
	}
}

void motion(int x, int y){
	ImGui_ImplGlut_MouseMotionCallback(x, y);
	if (bLeftPressed && x < nWindowWidth - nBarWidth)
	 {  // if left button is pressed
		nCurX = x;
		nCurY = y;
	}
}

void mouse(int button, int state, int x, int y)
{
	ImGui_ImplGlut_MouseButtonCallback(button, state);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		bLeftPressed = true;
		if (x < nWindowWidth - nBarWidth){
			nLastX = nCurX = x;
			nLastY = nCurY = y;
		}
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
		bRightPressed = true;
	}
	else {
		bLeftPressed = false;
		bRightPressed = false;
	}
}

void idle(){
	glutPostRedisplay();
}
void reshape(int w, int h){
	glViewport(0, nPolynomialHeight, w - nBarWidth, h - nPolynomialHeight);
	nWindowWidth = w;
	nWindowHeight = h;
}

void CompileAndLinkShader(){
	try {
		ModelShader.compileShader(vertex_shader.c_str(), GLSLShader::VERTEX);
		ModelShader.compileShader(fragment_shader.c_str(), GLSLShader::FRAGMENT);
		ModelShader.link();
		ModelShader.use();
	}
	catch (ShaderLibException & e) {
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
	for (int i = 0; i < 4;i++){
		CreateBuffer(ModelShader.getProgram(),vao[i],vbo[i],ibo[i]);
	}	
}
Drawer::Drawer(int* argc, char** argv){

	/* Initialize the GLUT window */
	glutInit(argc, argv);
	
	glutInitWindowSize(nWindowWidth, nWindowHeight);
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
	glClearColor(colBackground.x, colBackground.y, colBackground.z, 1.0f);
	GetPolyData();
	assert(pData);
	m_pEvaluator = nullptr;
	m_pMmarching = nullptr;		
}

bool Drawer::SetMarch(Marching* m){
	m_pMmarching = m;
	return true;
}

bool Drawer::GetPolyData()
{	
	if (m_pMmarching)
		pData = m_pMmarching->get_poly_data();
	if (pData)
		return true;
	else
		return false;
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
	if (m_pMmarching)
		m_pMmarching->reset_all_data();
}

void Drawer::ResetStep(){
	if (m_pMmarching)
		m_pMmarching->reset_step();
}

void Drawer::LoadFile()
{
	if (m_pMmarching)
		m_pMmarching->load_poly_from_file();
}

void Drawer::SaveFile()
{
	if (m_pMmarching)
		m_pMmarching->save_poly_to_file();
}

void Drawer::SetSeedMode(bool bSeed)
{
	if (m_pMmarching)
		m_pMmarching->seed_mode(bSeed);
}

bool Drawer::SetSeed(float fSeed[3])
{
	if (m_pMmarching)
		return m_pMmarching->set_seed(fSeed[0], fSeed[1], fSeed[2]);
	else
		false;
}

void Drawer::SetEvaluator(Evaluator* e){
	m_pEvaluator = e;
}

void Drawer::SetSurfaceConstant(float fConstant)
{
	if (m_pMmarching)
		m_pMmarching->set_surface_constant(fConstant);
}

void Drawer::SetSurfaceRepeatStepDistance(float fDistance)
{
	if (m_pMmarching)
		m_pMmarching->set_surface_repeat_step_distance(fDistance);
}

void Drawer::SetRepeatingSurfaceMode(bool bRepeat)
{
	if (m_pMmarching)
		m_pMmarching->repeating_surface_mode(bRepeat);
}

