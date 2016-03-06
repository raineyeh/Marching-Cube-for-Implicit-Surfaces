#ifndef MARCHING_531_H
#define MARCHING_531_H

#include <iostream>
#include <vector>
#include "evaluator.h"

struct Step_Data{
	int step_i; //indicates which step this is at. 0~n*n: step count down. -1: finished, -2:not started
	std::vector<float> corner_coords; //coordinate of the 4 vertices of the square. size=12
	std::vector<float> corner_values; //values of the 4 corners. size=4
	std::vector<float> intersect_coord; //intersection lines. size=2 if 1 line, 4 if 2 lines
};

struct Poly_Data{
	std::vector<float> vertex_list; //xyz coords
	std::vector<unsigned int> tri_list; //v1v2v3 list. v1v2 if it's a line
	Step_Data step_data;
};



class Marching
{
public:
	Marching(void);
	bool set_evaluator(Evaluator*);
	bool set_grid_step_size(float);	// must be between [0.001,0.5]. Changing this will reset step in step_by_step_mode.
	void step_by_step_mode(bool);
	void reset_step();
	Poly_Data const * get_poly_data();
	bool recalculate();
	

private:
	
	float evaluate(float x, float y, float z);
	void do_square(float, float, float, float);
	int add_line(float, float, float, float);
	int add_line(int, int);
	int add_point(float, float, float);
	void add_step_to_poly_data();


	Evaluator* evaluator;
	float grid_step_size;
	Poly_Data poly_data;
	bool is_step_by_step;

};

#endif