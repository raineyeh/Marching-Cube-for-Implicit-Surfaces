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

	bool Open(Poly_Data*){ return false; }
    bool Save(const Poly_Data*);
	Poly_Data* Open();

	Poly_Data poly_data;
private:
	
};
/*
static DWORD rgbCurrent;
static  COLORREF acrCustClr[16];
CHOOSECOLOR ccColour = { 0 };
ccColour.lStructSize = sizeof(ccColour);
ccColour.hwndOwner = GetForegroundWindow();
ccColour.lpCustColors = (LPDWORD)acrCustClr;
ccColour.rgbResult = rgbCurrent;
ccColour.Flags = CC_FULLOPEN | CC_RGBINIT;
if (ChooseColor(&ccColour) == TRUE)
{	
	rgbCurrent = ccColour.rgbResult;
}*/