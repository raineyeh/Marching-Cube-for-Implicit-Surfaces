/* The Marching class generates the triangular mesh that approximates an implicit surface.
* It contains the data structure that stores the surface mesh, and information during 1 step of surface calculation of a grid.
*/

#pragma once

#include "evaluator.h"
#include <vector>
#include <tuple>
#include <deque>
#include <set>
#include <math.h>

/* Step_Data is a structure that contains information for 1 grid cell's surface generation */
struct Step_Data{
	int step_i; //indicates which step this is at. counts from 1 up to n*n. -1: finished. -2:not started.
	std::vector<float> corner_coords; //coordinate of the 8 vertices of the cube. size=24
	std::vector<float> corner_values; //values of the 8 corners. size=8
	std::vector<float> intersect_coord; //intersection coordinate for each edge. size = 3* number of intersect points
	std::vector<int> tri_vlist; //vertex list of the triangles. size = 3*num_triangles. max_num_tri = 5
	std::vector<int> edge_list; // indices of the edges that has an intersection on it. size = 3 * number of intersections
	float surf_constant; //value of the surface. marching cube corner is considered positive if corner_values[i] > surf_constant
};

/* Poly_Data stores the mesh information of the implicit surface */
struct Poly_Data{
	std::vector<float> vertex_list; // point xyz coordinates. size = 3*num_points
	std::vector<unsigned int> tri_list; // list of triangle vertex indices, size = num_triangles * 3
	Step_Data step_data; // data of 1 step of grid evaluation, used when show step-by-step is checked by users.
};

/* xya is used to store a point location in 3d space, used for seeding calculation (only...) */
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
		//return std::tie(x, y, z) < std::tie(rhs.x, rhs.y, rhs.z); //too precise
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
	}
};

// Data structure for user input constraints. In the format of [equation] [op] [number], ex. x+y > 0
enum Comp_Op{ GT, LT, GE, LE, NAO }; // '>', '<', '>=', '<=', not-an-operation
Comp_Op parse_comp_op(string op);
struct Constraint{
	Constraint(){ in_use = valid = false; op = Comp_Op::NAO; };
	Constraint(string l, string o, float r) :lhs(l), rhs(r){ in_use = valid = false; op = parse_comp_op(o); };
	bool in_use;
	bool valid;
	string lhs; // the left hand side of the constraint. ie. the equation
	Comp_Op op; // the comparative opration. <, >, >=, <=
	float rhs;  // the number on the right hand side
	Evaluator eval;
};


class Marching
{
public:
	Marching(void); //default constructor
	bool set_evaluator(Evaluator*); //Evaluator contains the input equation and handles the evaluation

	Poly_Data const * get_poly_data(); //Returns the Poly_Data structure that contains the surface mesh information
	deque<xyz> const * get_seed_queue(); //Returns a list of grid cells waiting to be processed

	bool recalculate(); //Create the surface mesh and save it in poly_data. 1 step at a time if step_by_step_mode = on
	void reset_all_data(); //Clears all mesh data and step data. 
	
	bool set_grid_step_size(float);	//must be within [0.001,0.5]. Changing this will reset step in step_by_step_mode.
	float get_grid_size(){ return this->grid_step_size; };

	void step_by_step_mode(bool); //turn on/off step-by-step viewing
	void reset_step(); //start from the beginning of steps

	void set_surface_constant(float); //Defines at what value the surface is drawn. Default to 0

	/* For seeding, which is chosing which grid cell to start evaluation from */
	void seed_mode(bool); // turn on/off seeding mode. 
	bool set_seed(float x, float y, float z);   // set seed location. must be inside [-1,1]. The grid cell that this point is in will be the first to be evaluated. 
	void get_seed(float *x, float *y, float *z); // returns the seed location, which is 0,0,0 by default.

	/* Not used */
	bool set_surface_repeat_step_distance(float); //must be positive, and best not too low, depending on grid size.
	bool repeating_surface_mode(bool);

	/* set extra constraints. eg. set_extra_constraint1("z^2",">=",0.0) for the constraint z^2 >= 0.0 
	possible values for string op are {'>', '<', '>=', '<='}
	max number of constraint is 3
	Grid cells that are outside of the enabled constraints will not be evaluated */
	bool set_constraint0(string lhs, string op, float rhs) { return set_constraint(0, lhs, op, rhs); };
	bool set_constraint1(string lhs, string op, float rhs) { return set_constraint(1, lhs, op, rhs); };
	bool set_constraint2(string lhs, string op, float rhs) { return set_constraint(2, lhs, op, rhs); };
	bool set_constraint(int constraint_i, string constraint_lhs, string compare_op, float rhs_value);
	//toggle constraints use on or off
	bool use_constraint0(bool b){ return use_constraint(0, b); };
	bool use_constraint1(bool b){ return use_constraint(1, b); };
	bool use_constraint2(bool b){ return use_constraint(2, b); };
	bool use_constraint(int, bool);
	
	/* set scaling factor s_x,s_y,s_z for surface f(s_x*x,s_y*y,s_z*z)=t 
	they are default to 1.0 */
	void set_scaling_x(float);
	void set_scaling_y(float);
	void set_scaling_z(float);

	bool load_poly_from_file(); //load polygonal data from file in .ply format. result can be retrieved with get_poly_data()
	bool save_poly_to_file(); //save polygonal data to file in .ply format.
	

private:
	float evaluate(float x, float y, float z); //given x,y,z, evaluate it with the default evaluator
	float evaluate(Evaluator* e, float x, float y, float z); //evaluate a point using the given evaluator
	void calculate_step(float x, float y, float z); // construct marching cube surface for 1 grid cell with the specified lowerst corner coordinate
	int add_triangle(int, int, int); //add a triangle to Poly_Data
	int add_point(float, float, float); //add a point to Poly_Data, after checking existing points
	void add_step_to_poly_data(); //after calculating a grid cell, add the surface data to Poly_Data
	float interp(float x0, float x1, float v0, float v1); //return the location x where the linear interpolated value is surface constant
	void find_cubes_for_seeding(); //after a grid cell evaluation, put adjacent cells to evaluation queue. used in seeding mode
	bool check_constraints(float, float, float); // return true if the point is within constraints
	void print_step_info(); //print to console information on a grid cell evaluation, for debug purposes

	void get_starting_seed_grid(xyz*);  //to find where is the starting grid for the seed point 

	Evaluator* evaluator;
	float grid_step_size;
	Poly_Data poly_data;
	bool is_step_by_step;
	float surface_constant;
	float surface_step;
	bool is_repeating_surface;
	float scale_x, scale_y, scale_z;

	vector<Constraint> constraints; //a list of constraint equations to limit mesh generation.
	set<xyz> vertex_set; //a set with unique vertices in the mesh, so that vertices are not repeated in the mesh.

	bool is_seed_mode;
	float seed[3] ;
	//xyz seed_grid;
	deque <xyz> seed_queue;
	set<xyz> my_seed_set;

};

