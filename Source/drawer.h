#include "marching.h"
#include <string>
class Drawer{
public:
	Drawer(int*, char**);
	bool set_march(Marching*);
	void set_evaluator(Evaluator*);
	bool Get_poly_data();
	void start();
	void SetGridSize(float);
	void SetStepMode(bool);
	void SetEquation(string);
private:
	Marching* m_pMmarching;
	Evaluator* m_pEvaluator;
};