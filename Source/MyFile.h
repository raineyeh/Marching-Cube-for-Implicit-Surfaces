#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include "marching.h"
using namespace std;

class MyFile
{
public:
	MyFile();
	~MyFile();
	vector<int> Open();
	bool Save(const Poly_Data*);
private:
	
};

