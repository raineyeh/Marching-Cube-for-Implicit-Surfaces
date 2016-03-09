#ifndef MARCHING_531_H
#define MARCHING_531_H

#include <iostream>
#include <vector>
#include "evaluator.h"

struct Step_Data{
	int step_i; //indicates which step this is at. 0~n*n: step count down. -1: finished, -2:not started
	std::vector<float> corner_coords; //coordinate of the 8 vertices of the cube. size=24
	std::vector<float> corner_values; //values of the 8 corners. size=8
	std::vector<float> intersect_coord; //intersection coordinate for each edge. size = 3* number of intersect points
	std::vector<int> tri_vlist; //vertex list of the triangles. size = 3*num_triangles. max_num_tri = 5
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
	bool recalculate(); //update poly_data. 1 step at a time if step_by_step_mode = on
	

private:
	
	float evaluate(float x, float y, float z);
	void do_square(float, float, float, float, float, float);
	int add_line(float, float, float, float);
	int add_line(int, int);
	int add_triangle(int, int, int);
	int add_point(float, float, float);
	void add_step_to_poly_data();


	Evaluator* evaluator;
	float grid_step_size;
	Poly_Data poly_data;
	bool is_step_by_step;

};

#endif