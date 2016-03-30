#ifndef DRAW_H
#define DRAW_H

#include "marching.h"
#include <string>

class Drawer{
public:
	Drawer(int*, char**);
	bool SetMarch(Marching*);
	void SetEvaluator(Evaluator*);
	void SetSurfaceConstant(float);
	void SetSurfaceRepeatStepDistance(float);
	void SetRepeatingSurfaceMode(bool);
	bool GetPolyData();
	void Recalculate();
	void start();
	void SetGridSize(float);
	void SetStepMode(bool);
	void SetEquation(string);
	void ResetStep();	
	void LoadFile();
	void SaveFile();
private:
	Marching* m_pMmarching;
	Evaluator* m_pEvaluator;	
};


#endif