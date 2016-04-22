#pragma once

#include "evaluator.h"
#include <vector>
#include <tuple>
#include <deque>
#include <set>
#include <math.h>

struct Step_Data{
	int step_i; //indicates which step this is at. 0~n*n: step count down. -1: finished, -2:not started
	std::vector<float> corner_coords; //coordinate of the 8 vertices of the cube. size=24
	std::vector<float> corner_values; //values of the 8 corners. size=8
	std::vector<float> intersect_coord; //intersection coordinate for each edge. size = 3* number of intersect points
	std::vector<int> tri_vlist; //vertex list of the triangles. size = 3*num_triangles. max_num_tri = 5
	std::vector<int> edge_list;
	float surf_constant; //value of the surface. marching cube corner is considered positive if corner_values[i] > surf_constant

};

struct Poly_Data{
	std::vector<float> vertex_list; //xyz coords
	std::vector<unsigned int> tri_list; //v1v2v3 list. v1v2 if it's a line
	Step_Data step_data;
};

struct xyz{
	float x, y, z; int idx;
	xyz(){ x = y = z = NAN; idx = -1; };
	xyz(float xt, float yt, float zt, int i){ x = xt; y = yt; z = zt; idx = i; };
	xyz(float xt, float yt, float zt){ x = xt; y = yt; z = zt; idx = -1; };
	bool close_enough(float a, float b) const { 
		if (abs(a - b) < 0.000001) return true; 
		else return false; 
	}
	bool operator<(const xyz& rhs) const{
		if (!close_enough(x, rhs.x)){
			return x < rhs.x;
		}
		else if (!close_enough(y, rhs.y)){
			return y < rhs.y;
		}
		else if (!close_enough(z, rhs.z)){
			return z < rhs.z;
		}
		else return false;
		//return std::tie(x, y, z) < std::tie(rhs.x, rhs.y, rhs.z); //too precise
	}
};

enum Comp_Op{ GT, LT, GE, LE, NAO }; // '>', '<', '>=', '<=', not-an-operation

Comp_Op parse_comp_op(string op);

struct Constraint{
	Constraint(){ in_use = valid = false; op = Comp_Op::NAO; };
	Constraint(string l, string o, float r) :lhs(l), rhs(r){ in_use = valid = false; op = parse_comp_op(o); };
	bool in_use;
	bool valid;
	string lhs;
	Comp_Op op;
	float rhs;
	Evaluator eval;
};



class Marching
{
public:
	Marching(void);
	bool set_evaluator(Evaluator*);
	bool set_grid_step_size(float);	// must be between [0.001,0.5]. Changing this will reset step in step_by_step_mode.
	float get_grid_size(){ return this->grid_step_size; };
	void reset_all_data();
	Poly_Data const * get_poly_data();
	deque<xyz> const * get_seed_queue();
	bool recalculate(); //update poly_data. 1 step at a time if step_by_step_mode = on
	
	void step_by_step_mode(bool);
	void reset_step();

	void set_surface_constant(float); //Defines at what value the surface is drawn. Default to 0
	
	bool set_surface_repeat_step_distance(float); //must be positive, and best not too low, depending on grid size.
	bool repeating_surface_mode(bool);
    
	void seed_mode(bool);//this to set the seed mode to true or false 
	bool set_seed(float x, float y, float z);   //reading the seeed from the UI and checking 
	void get_seed(float *x, float *y, float *z);

	/* set extra constraints. eg. set_extra_constraint1("z^2",">=",0.0) for the constraint z^2 >= 0.0 
	possible values for string op are {'>', '<', '>=', '<='}
	max number of constraint is 3*/
	bool set_constraint0(string lhs, string op, float rhs) { return set_constraint(0, lhs, op, rhs); };
	bool set_constraint1(string lhs, string op, float rhs) { return set_constraint(1, lhs, op, rhs); };
	bool set_constraint2(string lhs, string op, float rhs) { return set_constraint(2, lhs, op, rhs); };
	bool use_constraint0(bool b){ return use_constraint(0, b); };
	bool use_constraint1(bool b){ return use_constraint(1, b); };
	bool use_constraint2(bool b){ return use_constraint(2, b); };
	bool use_constraint(int, bool);
	bool set_constraint(int constraint_i, string constraint_lhs, string compare_op, float rhs_value);

	/* set scaling factor s_x,s_y,s_z for surface f(s_x*x,s_y*y,s_z*z)=t 
	they are default to 1.0 */
	void set_scaling_x(float);
	void set_scaling_y(float);
	void set_scaling_z(float);

	bool load_poly_from_file(); //load polygonal data from file in .poly format. result can be retrieved with get_poly_data()
	bool save_poly_to_file(); //save polygonal data to file in .poly format.
	

private:
	float evaluate(float x, float y, float z);
	float evaluate(Evaluator* e, float x, float y, float z);
	void calculate_step(float, float, float);
	int add_triangle(int, int, int);
	int add_point(float, float, float);
	void add_step_to_poly_data();
	float interp(float, float, float, float);
	void find_cubes_for_seeding();
	bool check_constraints(float, float, float);
	void print_step_info();

	void get_starting_seed_grid(xyz*);  //to find where is the starting grid for the seed point 

	Evaluator* evaluator;
	float grid_step_size;
	Poly_Data poly_data;
	bool is_step_by_step;
	float surface_constant;
	float surface_step;
	bool is_repeating_surface;
	float scale_x, scale_y, scale_z;

	vector<Constraint> constraints;
	set<xyz> vertex_set;

	bool is_seed_mode;
	float seed[3] ;
	//xyz seed_grid;
	deque <xyz> seed_queue;
	set<xyz> my_seed_set;

};

