#include "MyFile.h"
#include <windows.h>
#include <iostream>
#include <vector>
#include "marching.h"
using namespace std;

MyFile::MyFile()
{		
}
MyFile::~MyFile()
{
}


 Poly_Data*  MyFile::Open()


{
	Poly_Data* data =NULL;
	FILE *fp;
	OPENFILENAME ofn;
	TCHAR	szFile[MAX_PATH] = "mesh.poly";
	TCHAR	szTitle[MAX_PATH] = "points";
	static TCHAR szFilter[] = "file type\0*.*\0file type1\0*.*0";
	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;//类型            
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = szFile;// 文件路径指针            
	ofn.nMaxFile = MAX_PATH;//文件路径大小            
	ofn.lpstrFileTitle = szTitle;//文件名指针     
	ofn.nMaxFileTitle = MAX_PATH;//文件名大小            
	ofn.lpstrInitialDir = NULL;//初始化路径            
	ofn.lpstrTitle = "Open";//对话框标            
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;//位标记的设置 
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("poly");
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if (GetOpenFileName(&ofn) == false) return NULL;// ask gordon
	errno_t err = fopen_s(&fp, szFilter, "r");
	if (NULL == fp) return  NULL; //ask gordon
	int num = 0; int tri_num = 0;
	float x = 0, y = 0, z = 0;
	fscanf_s(fp, "%d", &num);	                      
	for (int i = 0; i <num; i++)
	{
		fscanf_s(fp, "%f", &x);
		fscanf_s(fp, "%f", &y);		
		fscanf_s(fp, "%f", &z);
		data->vertex_list.push_back(x);
		data->vertex_list.push_back(y);
		data->vertex_list.push_back(z);
	}
	unsigned int x0, y0, z0;
	fscanf_s(fp, "%d", &tri_num);
	for (int i = 0; i <tri_num; i++)
	{
		for (int j = 1; j <= 3; j++)
		{
		fscanf_s(fp, "%u", &x0);
		fscanf_s(fp, "%u", &y0);
		fscanf_s(fp, "%u", &z0);
		data->tri_list.push_back(x);
		data->tri_list.push_back(y);
		data->tri_list.push_back(z);
		}
	}
 	fclose(fp);
	return data;
}

bool MyFile::Save(const Poly_Data* data)
{
	FILE *fp;
	OPENFILENAME ofn;
	TCHAR	szFile[MAX_PATH] = "*.poly";
	TCHAR	szTitle[MAX_PATH] = "points";
	static TCHAR szFilter[] = "file type\0*.*\0file type1\0*.*0";
	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;//类型            
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = szFile;// 文件路径指针            
	ofn.nMaxFile = MAX_PATH;//文件路径大小            
	ofn.lpstrFileTitle = szTitle;//文件名指针     
	ofn.nMaxFileTitle = MAX_PATH;//文件名大小            
	ofn.lpstrInitialDir = NULL;//初始化路径            
	ofn.lpstrTitle = "Save";//对话框标            
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;//位标记的设置 
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("poly");
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if (GetOpenFileName(&ofn) == false) return false;
	errno_t err = fopen_s(&fp, ofn.lpstrFile, "w");
	if (NULL == fp) return false;
	int num = 0; int tri_num = 0;
	float x = 0, y = 0, z = 0;
	num = ((data->vertex_list.size()) / 3);
	cout << num;
	char buffer[50];
	sprintf_s(buffer, "%d", num);
	fprintf_s(fp, "%s", buffer);
	fprintf_s(fp, "\r\n");
		for (int i = 0; i <num; i++)
		{
		x=data->vertex_list.at(i*3);
		y=data->vertex_list.at(i*3+1);
		z=data->vertex_list.at(i*3+2);
		sprintf_s(buffer, "%f %f %f ", x,y,z);
		fprintf_s(fp, "%s", buffer);
		fprintf_s(fp, "\r\n");
		}
		tri_num = ((data->tri_list.size()) / 3);
	unsigned int x0=0, y0=0, z0=0;
	sprintf_s(buffer, "%d", tri_num);
	fprintf_s(fp, "%s", buffer);
	fprintf_s(fp, "\r\n");
	for (int i = 0; i <tri_num; i++)
	{
		for (int j = 1; j <= 3; j++)
		{
			x0 = data->tri_list.at(i * 3);
			y0 = data->tri_list.at(i * 3 + 1);
			z0 = data->tri_list.at(i * 3 + 2);
			sprintf_s(buffer, "%f %f %f ", x, y, z);
			fprintf_s(fp, "%s", buffer);
			fprintf_s(fp, "\r\n");
		}
	}
	
	fclose(fp);
	return true;
}
