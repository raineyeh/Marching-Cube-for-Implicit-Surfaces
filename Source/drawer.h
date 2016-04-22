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
	void LoadEquation(string&);
	void SaveEquation();
	void ResetAlldata();
	bool GetPolyData();
	void Recalculate();
	void start();
	void SetGridSize(float);
	void SetStepMode(bool);
	void SetEquation(string);
	void ResetStep();	
	void LoadFile();
	void SaveFile();
	void SetSeedMode(bool);
	bool SetSeed(float[3]);
	void SetScaling(float[3]);
	bool SetConstraint0(string lhs, string op, float rhs);
	bool SetConstraint1(string lhs, string op, float rhs);
	bool SetConstraint2(string lhs, string op, float rhs);
	bool UseExtraConstraint0(bool b);
	bool UseExtraConstraint1(bool b);
	bool UseExtraConstraint2(bool b);
private:
	Marching* m_pMmarching;
	Evaluator* m_pEvaluator;	
};


#endif