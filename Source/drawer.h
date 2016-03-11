#include "marching.h"
#include <string>
#include <objbase.h>
class Drawer{
public:
	Drawer(int*, char**);
	bool set_march(Marching*);
	void set_evaluator(Evaluator*);
	bool GetPolyData();
	void Recalculate();
	void start();
	void SetGridSize(float);
	void SetStepMode(bool);
	void SetEquation(string);
	void ResetStep();
	HANDLE m_hEvent;
	
private:
	Marching* m_pMmarching;
	Evaluator* m_pEvaluator;		
};