#ifndef MARCHING_531_H
#define MARCHING_531_H

#include <iostream>
#include <vector>
#include <map>
#include "evaluator.h"

struct Poly_Data{
	std::vector<float> vertex_list; //xyz coords
	std::vector<unsigned short> tri_list; //v1v2v3 list. v1v2-1 if it's a line
};

class Marching
{
public:
	Marching(void);
	bool set_evaluator(Evaluator*);
	bool set_grid_step_size(float);
	Poly_Data const * get_poly_data();
	bool recalculate();
	

private:
	
	float evaluate(float x, float y, float z);
	void do_square(float, float, float, float);
	int add_line(float, float, float, float);
	int add_line(int, int);
	int add_point(float, float, float);


	Evaluator* evaluator;
	float grid_step_size;
	Poly_Data poly_data;
	bool dirty;

};

#endif