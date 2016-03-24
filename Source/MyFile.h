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
	Poly_Data Open();
	bool Save(const Poly_Data*);

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