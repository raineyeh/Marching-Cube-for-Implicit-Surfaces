#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
using namespace std;

class MyFile
{
public:
	MyFile();
	~MyFile();
	vector<int> Open();
	bool Save(vector<int>);
private:
	
};

