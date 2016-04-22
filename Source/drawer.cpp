#define GLM_FORCE_RADIANS
#include "drawer.h"
#include "imgui_impl_glut.h" 
#include <algorithm>
#include <memory> 
#include <thread>
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
#include "vao.h"
#include "MyFile.h"
using namespace std;

/* Window information */
int nPolynomialHeight = 70;
int nBarWidth = 320;
int nBase;
int nWindowHeight;
int nWindowWidth;
int windowID = -1;
float nColor = 48;
/* Data information */
const Poly_Data* pData;
static const std::string vertex_shader("..\\..\\Source\\vs.glsl");
static const std::string fragment_shader("..\\..\\Source\\fs.glsl");
static char szEquation[256] = "x+y";//(x^2+y^2-1)^2 + (x^2+z^2-1)^2 + (z^2+y^2-1)^2 - 0.5
static float fGrid = 0.2f;
static float fSurfaceConstant = 0.0f;
static float fStepDistance = 0.5f;
static float fPercent;
static float fScaler[3] = { 1.1f, 1.1f, 1.1f };
static float fSeeding[3] = {0.8f,0.8f,0.9f};
static int nMovieSpeed = 5;
static bool bStepMode,bSeedingMode, bTranslucent, bInvertFace, bRepeatingMode;
static ImVec4 colBackground = ImColor(1.0f, 1.0f,1.0f, 1.0f);
static ImVec4 colModelFrontFace = ImColor(1.0f, 0.3f, 0.3f, 1.0f);
static ImVec4 colModelBackFace = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
static ImVec4 colCubeSurface = ImColor(0.7f, 0.7f, 0.7f, 0.5f);
static ImVec4 colCubeEdge = ImColor(1.0f, 0.0f, 1.0f, 1.0f);
static ImVec4 colModelEdge = ImColor(0.0f, 0.0f,0.0f, 1.0f);
static ImVec4 colIntersectPoint = ImColor(0.0f, 0.0f, 1.0f, 1.0f);
static ImVec4 colIntersectSurface = ImColor(0.0f, 1.0f, 1.0f, 1.0f);
static ImVec4 colIntersectSurfaceEdges = ImColor(0.0f, 0.0f, 1.0f, 1.0f);
static ImVec4 colInCornerPoint = ImColor(0.0f, 1.0f, 0.0f, 1.0f);
static ImVec4 colOutCornerPoint = ImColor(1.0f, 0.0f, 0.0f, 1.0f);
string op[4] = { ">=", "<=", "<", ">" };
bool bLeftPressed, bRightPressed, bHasInit, bMovie, bPause, bCubeStep;
int nLastX, nLastY, nCurX, nCurY;
int nCubeStep;
int nLineWidth = 3;
GLuint vao[3], vbo[3], ibo[3];//0 for model, 1 for cube, 2 for intersect
glm::mat4 M,V,P;
vector<float> vIntersectVertex, vIntersectIndex;
//combo
static char szlhs1[256], szlhs2[256], szlhs3[256];
static bool bConstraint1, bConstraint2, bConstraint3;
static float frhs1, frhs2, frhs3;
static int item1, item2, item3;
// objects 
MyFile myfile;
Drawer* pDrawer = nullptr;
ShaderLib ModelShader;
thread movie;

//cube idx
unsigned int idxCube[36] = { 1, 0, 2, 2, 0, 3, 0, 5, 4, 5, 0, 1, 1, 6, 5, 6, 1, 2, 2, 7, 6, 7, 2, 3,3, 0, 7, 0, 4, 7, 4, 5, 6, 6, 7, 4 };
unsigned int idxEdge[30] = { 0, 1, 2, 3, 0, 1, 5, 6, 2, 1, 5, 4, 7, 6, 5, 4, 0, 3, 7, 4, 4, 5, 1, 0, 4, 7, 3, 2, 6, 7 };

void BufferData(GLuint ibo, GLuint ni, void* pi, GLuint vbo, GLuint nv, void* pv){
	glNamedBufferData(ibo, ni, pi, GL_DYNAMIC_DRAW);
	glNamedBufferData(vbo, nv, pv, GL_DYNAMIC_DRAW);	
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

static int Movie()
{		
	if (pDrawer == nullptr) return false;
	if (pData == nullptr) return false;
	float fTotalStep = (float)pData->step_data.step_i;
	
	for (int i = 0; bMovie && i != pData->step_data.step_i;){
		if (bPause) {
			nCubeStep = 3;
			continue;
		}
		nCubeStep = -1;
		fPercent = pData->step_data.step_i / fTotalStep;
		fPercent = 1.0f - fPercent;
		pDrawer->Recalculate();	
		if (pData->step_data.intersect_coord.size()>0)
		for (int nStep = 0; nStep < 4; nStep++)
		{
			nCubeStep++;
			Sleep(200+(5 - nMovieSpeed) * 10);
		}
		nCubeStep = -1;
		Sleep(50 + (5 - nMovieSpeed)*10);
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
	if (pDrawer == nullptr) return;
	ImGui_ImplGlut_NewFrame("Marching Cube",0.5f);
	ImVec2 wsize(nBarWidth, nWindowHeight - nPolynomialHeight);
	ImGui::SetWindowFontScale(1.5);
	ImGui::SetWindowSize(wsize);
	ImVec2 wpos(nWindowWidth - nBarWidth, nPolynomialHeight);
	ImGui::SetWindowPos(wpos);
	
	if (!bMovie && ImGui::SliderFloat("Grid size", &fGrid, 0.01f, 0.5f,"%.2f")){		
		pDrawer->SetGridSize(fGrid);
		pDrawer->ResetStep();					
	}
	ImGui::SliderInt("Movie speed", &nMovieSpeed, 1, 9);		
	if (ImGui::Checkbox("Translucent", &bTranslucent)){
		if (bTranslucent){
			glDisable(GL_DEPTH_TEST);			
			glEnable(GL_BLEND);
			colModelFrontFace.w = 0.5f;
			nLineWidth = 1;
		}
		else{
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			colModelFrontFace.w = 1.0f;
			nLineWidth = 3;
		}		
		ModelShader.setUniform("uTranslucent", bTranslucent);
	}
	if (!bMovie && ImGui::Checkbox("Step mode", &bStepMode)){
		pDrawer->SetEquation(string(szEquation));
		pDrawer->GetPolyData();		
		for (int i = 0; i < 3; i++)
			BufferData(ibo[i], 0, 0, vbo[i], 0, 0);		
		pDrawer->ResetStep();
		pDrawer->SetStepMode(bStepMode);					
	}
	if (!bSeedingMode && bStepMode){
		char ch[20] = { 0 };
		if (pData && bStepMode && pData->step_data.step_i >= 0)
			sprintf_s(ch, "RemainSteps:%d", pData->step_data.step_i);
		ImGui::Text(ch);
	}
	if (!bMovie && ImGui::Checkbox("Seeding mode", &bSeedingMode)){				
		pDrawer->SetSeedMode(bSeedingMode);
		pDrawer->SetSeed(fSeeding);
	}	
	if (!bMovie && bSeedingMode && ImGui::InputFloat3("Seeding point", fSeeding, 2))
		pDrawer->SetSeed(fSeeding);
	
	if (!bMovie && ImGui::Checkbox("Repeating surface mode", &bRepeatingMode)){		
		pDrawer->SetRepeatingSurfaceMode(bRepeatingMode);
		pDrawer->SetSurfaceRepeatStepDistance(fStepDistance);
		pDrawer->ResetStep();				
	}
/*	if (!bMovie && bRepeatingMode && ImGui::SliderFloat(".", &fStepDistance, 0.1f, 0.9f, "%.2f")){
		pDrawer->SetSurfaceRepeatStepDistance(fStepDistance);
		pDrawer->ResetStep();
	}
	if (!bMovie && bRepeatingMode)ImGui::Text("Surface repeat step distance");
*/	
	if (bMovie){
		ImVec2 psize(nBarWidth, 30);
		ImGui::ProgressBar(fPercent, psize);
	}		  			

	//Constraint1
	if (!bMovie && ImGui::Checkbox("Constraint1", &bConstraint1)){
		pDrawer->UseExtraConstraint0(bConstraint1);
	}
	if (!bMovie && bConstraint1 && ImGui::InputText("Equation1", szlhs1, 256, 0))
		pDrawer->SetConstraint0(szlhs1, op[item1], frhs1);
	if (!bMovie && bConstraint1 && ImGui::Combo("operation", &item1, ">=\0<=\0<\0>\0"))
		pDrawer->SetConstraint0(szlhs1, op[item1], frhs1);
	if (!bMovie && bConstraint1 && ImGui::InputFloat("Const1", &frhs1, 0.0f, 0.0f, 2))
		pDrawer->SetConstraint0(szlhs1, op[item1], frhs1);
	
	//Constraint2
	if (!bMovie && ImGui::Checkbox("Constraint2", &bConstraint2)) {
		pDrawer->UseExtraConstraint1(bConstraint1);
	}
	if (!bMovie && bConstraint2 && ImGui::InputText("Equation2", szlhs2, 256, 0))
		pDrawer->SetConstraint0(szlhs2, op[item2], frhs2);
	if (!bMovie && bConstraint2 && ImGui::Combo("operation", &item2, ">=\0<=\0<\0>\0"))
		pDrawer->SetConstraint0(szlhs2, op[item2], frhs2);
	if (!bMovie && bConstraint2 && ImGui::InputFloat("Const2", &frhs2, 0.0f, 0.0f, 2))
		pDrawer->SetConstraint0(szlhs2, op[item2], frhs2);

	//Constraint3
	if (!bMovie && ImGui::Checkbox("Constraint3", &bConstraint3)) {
		pDrawer->UseExtraConstraint2(bConstraint1);
	}
	if (!bMovie && bConstraint3 && ImGui::InputText("Equation3", szlhs3, 256, 0))
		pDrawer->SetConstraint0(szlhs3, op[item3], frhs3);
	if (!bMovie && bConstraint3 && ImGui::Combo("operation", &item3, ">=\0<=\0<\0>\0"))
		pDrawer->SetConstraint0(szlhs3, op[item3], frhs3);
	if (!bMovie && bConstraint3 && ImGui::InputFloat("Const3", &frhs3, 0.0f, 0.0f, 2))
		pDrawer->SetConstraint0(szlhs3, op[item3], frhs3);

	
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
		pDrawer->LoadFile();
		pDrawer->ResetStep();
		pDrawer->GetPolyData();
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
		ImGui::ColorPlate(&colIntersectSurfaceEdges, "Intersect edges color");		
		ImGui::EndPopup();
	}
	
	//top bar	
	ImGui::Begin("ImGui Demo", 0.72f, false, ImVec2(nWindowHeight, 0), -1.0f, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	ImVec2 psize(nWindowWidth, nPolynomialHeight);	
	ImGui::SetWindowSize(psize);
	ImVec2 ppos(0,0);
	ImGui::SetWindowPos(ppos);
	ImGui::Text("P(ax,by,cz)="); ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5);
	ImGui::InputText("=", szEquation, 256, 0);ImGui::SameLine();	
	if (ImGui::InputFloat("s", &fSurfaceConstant, 0.0f, 0.0f, 2)){
		pDrawer->SetSurfaceConstant(fSurfaceConstant);
		pDrawer->ResetStep();		
	}
	ImGui::Text("a b c = "); ImGui::SameLine();
	if (ImGui::InputFloat3(" ", fScaler)){		
		pDrawer->SetScaling(fScaler);
	}	
	ImGui::SameLine();
	
	if (!bMovie && ImGui::Button("Refresh")){
		if (!bStepMode){
			pDrawer->SetEquation(string(szEquation));
			pDrawer->SetGridSize(fGrid);
			pDrawer->Recalculate();
			pDrawer->GetPolyData();
			if (pData && !pData->tri_list.empty() && !pData->vertex_list.empty())
				BufferData(ibo[0], pData->tri_list.size()*sizeof(unsigned int), (void*)&pData->tri_list[0],
				vbo[0], pData->vertex_list.size()*sizeof(float), (void*)&pData->vertex_list[0]);
		}
		else{
			if (bCubeStep){
				nCubeStep++;
				if (nCubeStep == 4) {
					nCubeStep = 0;
					pDrawer->Recalculate();
				}				
			}
			else
				pDrawer->Recalculate();
			bCubeStep = pData->step_data.intersect_coord.size() > 0 ? true : false;
		}
		SetStepData();		
	} ImGui::SameLine();
	if (!bMovie && ImGui::Button("Reset")){		
		pDrawer->ResetStep();
		for (int i = 0; i < 3; i++)
			BufferData(ibo[i], 0, 0, vbo[i], 0, 0);
	}ImGui::SameLine();
	if (ImGui::Button(bMovie ? "Stop" : "Movie")){
		if (bMovie){
			bMovie = false;
			bPause = false;
			movie.join();
			for (int i = 0; i < 3; i++)
				BufferData(ibo[i], 0, 0, vbo[i], 0, 0);
		}
		else{
			bStepMode = true;
			bMovie = true;
			bPause = false;
			pDrawer->ResetStep();
			pDrawer->SetStepMode(true);
			pDrawer->SetEquation(string(szEquation));
			pDrawer->SetGridSize(fGrid);
			pDrawer->Recalculate();
			pDrawer->GetPolyData();
			movie = thread(Movie);
		}		
	}
	if (bMovie){
		ImGui::SameLine();
		if (ImGui::Button(bPause ? "Resume" : "Pause")){
			if (bPause)
				bPause = false;
			else
				bPause = true;
		}
	}
	ImGui::End();	
// 	static bool show2 = true;
// 	ImGui::ShowTestWindow(&show2);
	ImGui::Render();
	bHasInit = true;	
}
glm::vec3 GetArcballVector(int x, int y) {
	glm::vec3 P = glm::vec3(1.0*x / nWindowWidth * 2 - 1.0,
		1.0*y / nWindowHeight * 2 - 1.0,0);//屏幕坐标系变为[-1,1]
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
	glEnable(GL_BLEND);
	glDepthMask(false);	
	BufferData(ibo[1], sizeof(idxCube), idxCube, 0, 0, 0);//修改数据后要bind
	ModelShader.setUniform("uFrontColor", glm::vec4(colCubeSurface.x, colCubeSurface.y, colCubeSurface.z, 0.5f));
	ModelShader.setUniform("uBackColor", glm::vec4(colCubeSurface.x, colCubeSurface.y, colCubeSurface.z, 0.5f));
	glBindVertexArray(vao[1]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glDepthMask(true);
	if (!bTranslucent)
		glDisable(GL_BLEND);

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
	//intersect	point
	if (nCubeStep > 1){
		glBindVertexArray(vao[2]);
		ModelShader.setUniform("uFrontColor", glm::vec4(colIntersectPoint.x, colIntersectPoint.y, colIntersectPoint.z, 1.0f));
		glDrawArrays(GL_POINTS, 0, pData->step_data.intersect_coord.size() / 3);
	}
	glDepthMask(true);
	
	//polygon	
	if (pData->step_data.tri_vlist.size() > 0){	
		BufferData(ibo[2], pData->step_data.tri_vlist.size()*sizeof(unsigned int), (void*)&pData->step_data.tri_vlist[0], 0, 0, 0);//修改数据后要bind
		glBindVertexArray(vao[2]);
		if (nCubeStep > 2){
			ModelShader.setUniform("uFrontColor", glm::vec4(colIntersectSurface.x, colIntersectSurface.y, colIntersectSurface.z, 1.0f));
			ModelShader.setUniform("uBackColor", glm::vec4(colIntersectSurface.x, colIntersectSurface.y, colIntersectSurface.z, colModelBackFace.w));
			glDrawElements(GL_TRIANGLES, pData->step_data.intersect_coord.size(), GL_UNSIGNED_INT, 0);
			//polygon edges
			glLineWidth(4);
			ModelShader.setUniform("uFrontColor", glm::vec4(colIntersectSurfaceEdges.x, colIntersectSurfaceEdges.y, colIntersectSurfaceEdges.z, 1.0f));
			glDrawElements(GL_LINES, pData->step_data.intersect_coord.size(), GL_UNSIGNED_INT, 0);
			glLineWidth(2);
		}
	}

	glBindVertexArray(0);	
}

void DrawModel(){		
	if (pData == nullptr) return;
	
	//mesh
	glBindVertexArray(vao[0]); 	
	ModelShader.setUniform("uFrontColor", glm::vec4(colModelFrontFace.x, colModelFrontFace.y, colModelFrontFace.z, bTranslucent ? 0.5f : 1.0f));
	ModelShader.setUniform("uBackColor", glm::vec4(colModelBackFace.x, colModelBackFace.y, colModelBackFace.z, colModelBackFace.w));
	glDrawElements(GL_TRIANGLES, (pData->vertex_list.size() - 2) * 3, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_POINTS, 0,(pData->vertex_list.size() - 2) * 3);
	
	//edges
	glm::mat4 M_old = M;
	M = glm::scale(M, glm::vec3(1.001f));
	ModelShader.setUniform("M", M);	
	glLineWidth(nLineWidth);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	
	ModelShader.setUniform("uFrontColor", glm::vec4(colModelEdge.x, colModelEdge.y, colModelEdge.z, 1.0f));
	ModelShader.setUniform("uBackColor", glm::vec4(colModelEdge.x, colModelEdge.y, colModelEdge.z, 1.0f));
	glDrawElements(GL_TRIANGLES, (pData->vertex_list.size() - 2) * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(2);
	M = M_old;
	ModelShader.setUniform("M", M);	
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
	if (pDrawer && key == '\r'){
		pDrawer->SetEquation(string(szEquation));
		pDrawer->SetGridSize(fGrid);
		pDrawer->Recalculate();
		pDrawer->GetPolyData();
		if (pData && !pData->tri_list.empty() && !pData->vertex_list.empty())
			BufferData(ibo[0], pData->tri_list.size()*sizeof(unsigned int), (void*)&pData->tri_list[0],
			vbo[0], pData->vertex_list.size()*sizeof(float), (void*)&pData->vertex_list[0]);
	}
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
	int nMaxLenth = h - nPolynomialHeight;
	glViewport((w - nBarWidth - nMaxLenth) / 2.0f, 0, nMaxLenth, nMaxLenth);
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
	for (int i = 0; i < 3;i++){
		CreateBuffer(ModelShader.getProgram(), vao[i], vbo[i], ibo[i]);
	}	
}
Drawer::Drawer(int* argc, char** argv){

	/* Initialize the GLUT window */
	glutInit(argc, argv);
	
	int nResX = GetSystemMetrics(SM_CXSCREEN);
	int nResY = GetSystemMetrics(SM_CYSCREEN);	
	nWindowHeight = nResY - 100;
	nBase = nWindowHeight - nPolynomialHeight;	
	nWindowWidth = nBase + nBarWidth;
	glutInitWindowSize(nWindowWidth, nWindowHeight);
	glutInitWindowPosition((nResX - nWindowWidth)/2, 0);
	glutSetOption(GLUT_MULTISAMPLE, 4); //Set number of samples per pixel
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	windowID = glutCreateWindow("Marching Cube");

	glewInit();
	ImGui_ImplGlut_Init();
	CompileAndLinkShader();

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
	SetScaling(fScaler);
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
		return false;
}

void Drawer::SetScaling(float fScale[3])
{
	if (m_pMmarching){
		m_pMmarching->set_scaling_x(fScale[0]);
		m_pMmarching->set_scaling_y(fScale[1]);
		m_pMmarching->set_scaling_z(fScale[2]);
	}		
}

bool Drawer::SetConstraint0(string lhs, string op, float rhs)
{
	if (m_pMmarching)
		return m_pMmarching->set_constraint0(lhs,  op,  rhs);
	return false;
}

bool Drawer::SetConstraint1(string lhs, string op, float rhs)
{
	if (m_pMmarching)
		return m_pMmarching->set_constraint1(lhs, op, rhs);
	return false;
}

bool Drawer::SetConstraint2(string lhs, string op, float rhs)
{
	if (m_pMmarching)
		return m_pMmarching->set_constraint2(lhs, op, rhs);
	return false;
}

bool Drawer::UseExtraConstraint0(bool b)
{
	if (m_pMmarching)

		return m_pMmarching->use_constraint0(b);
	return false;
}

bool Drawer::UseExtraConstraint1(bool b)
{
	if (m_pMmarching)
		return m_pMmarching->use_constraint1(b);
	return false;
}

bool Drawer::UseExtraConstraint2(bool b)
{
	if (m_pMmarching)
		return m_pMmarching->use_constraint2(b);
	return false;
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

