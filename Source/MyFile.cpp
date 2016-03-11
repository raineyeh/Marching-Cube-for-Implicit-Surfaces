#include "MyFile.h"

MyFile::MyFile()
{		
}
MyFile::~MyFile()
{
}

vector<int> MyFile::Open()
{
	vector<int> vPoint;	
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

	if (GetOpenFileName(&ofn) == false) return vPoint;
	errno_t err = fopen_s(&fp, szFilter, "r");
	if (NULL == fp) return vPoint;

	int num = 0;
	float x = 0, y = 0;
	
	fscanf_s(fp, "%d", &num);	                      
	for (int i = 0; i <num; i++)
	{
		fscanf_s(fp, "%f", &x);
		fscanf_s(fp, "%f", &y);		
	}
 	fclose(fp);
	return vPoint;
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
	
	
	fclose(fp);
	return true;
}
