#ifndef MARCHING_531_H
#define MARCHING_531_H

#include <iostream>
#include <vector>
#include <tuple>
#include "evaluator.h"
#include <queue>
#include <set>

struct Step_Data{
	int step_i; //indicates which step this is at. 0~n*n: step count down. -1: finished, -2:not started
	std::vector<float> corner_coords; //coordinate of the 8 vertices of the cube. size=24
	std::vector<float> corner_values; //values of the 8 corners. size=8
	std::vector<float> intersect_coord; //intersection coordinate for each edge. size = 3* number of intersect points
	std::vector<int> tri_vlist; //vertex list of the triangles. size = 3*num_triangles. max_num_tri = 5
	std::vector<int> edge_list;
	float surf_constant; //value of the surface. marching cube corner is considered positive if corner_values[i] > surf_constant

};
struct vertex_list
{
	float x0, x1, y0,  y1, z0, z1;
	bool operator<(const vertex_list&rhs) const {
		if (x0 == rhs.x0) 
		{
			if (y0 == rhs.y0)
			{
				return z0 < rhs.z0;
			}
			else
				return y0 < rhs.y0;
		}
		else
			return x0 < rhs.x0;
	   }
	bool operator>(const vertex_list&rhs) const {
		if (x0 == rhs.x0)
		{
			if (y0 == rhs.y0)
			{
				return z0 > rhs.z0;
			}
			else
				return y0 > rhs.y0;
		}
		else
			return x0 > rhs.x0;
	}
	bool operator==(const vertex_list&rhs) const {
		if ((x0 == rhs.x0) && (y0 == rhs.y0 )&& (z0 == rhs.z0))
			return true;
		else
			return false;
		
	}
};

struct Poly_Data{
	std::vector<float> vertex_list; //xyz coords
	std::vector<unsigned int> tri_list; //v1v2v3 list. v1v2 if it's a line
	Step_Data step_data;
};

struct xyz{
	float x, y, z; int idx;
	xyz(float xt, float yt, float zt, int i){ x = xt; y = yt; z = zt; idx = i; };
	xyz(float xt, float yt, float zt){ x = xt; y = yt; z = zt; idx = -1; };
	bool operator<(const xyz& rhs) const{
		return std::tie(x, y, z) < std::tie(rhs.x, rhs.y, rhs.z);
	}
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
	void set_surface_constant(float); //Defines at what value the surface is drawn. Default to 0
	bool set_surface_repeat_step_distance(float); //must be positive, and best not too low, depending on grid size.
	bool repeating_surface_mode(bool);
    void seed_mode(bool);//this to set the seed mode to true or false 
	bool set_seed(float x, float y, float z);   //reading the seeed from the UI and checking 
	void get_seed_grid();  //to find where is the starting grid for the seed point 
private:
			
	float evaluate(float x, float y, float z);
	void do_square(float, float, float, float, float, float);
	int add_triangle(int, int, int);
	int add_point(float, float, float);
	void add_step_to_poly_data();
	float interp(float, float, float, float);
	void find_cubes_for_seeding();
	void print_step_info();

	Evaluator* evaluator;
	float grid_step_size;
	Poly_Data poly_data;
	bool is_step_by_step;
	float surface_constant;
	float surface_step;
	bool is_repeating_surface;

	set<xyz> vertex_set;

	bool is_seed_mode;
	float seed[3] ;
	vertex_list seed_grid;
	bool cubes[26];
	queue <vertex_list> seed_queue; 
	set<vertex_list> my_seed_set;
};

#endif