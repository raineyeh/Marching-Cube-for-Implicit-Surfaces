#include "marching.h"
#include "marching_lookup.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <math.h>
using namespace std;

// Print information for a single grid cell evaluation. For debug only.
void Marching::print_step_info(){
	Step_Data*sd = &this->poly_data.step_data;
	cout << "x_0:" << sd->corner_coords[0] << " y_0:" << sd->corner_coords[1] << " z_0:" << sd->corner_coords[2] << endl;
	int c_code = 0;
	for (int i = 0; i < 8; i++){
		bool b = (sd->corner_values[i] > sd->surf_constant);
		cout << "corner " << i << "(" << (b?1:0) << "):" << sd->corner_values[i] << endl;
		c_code += b ? two_to_the[i]: 0;
	}
	cout << "cube_code:" << c_code << endl;
}

//Constructor
Marching::Marching(void){
	this->grid_step_size = (float)0.25;
	this->evaluator = NULL;
	this->poly_data.step_data.corner_coords.resize(24);
	this->poly_data.step_data.corner_values.resize(8);
	this->poly_data.step_data.step_i = -2;
	this->is_step_by_step = false;
	this->is_repeating_surface = false;
	this->surface_constant = 0;
	this->surface_step = 0;
	this->is_seed_mode = false;
	seed[0] = 0; seed[1] = 0; seed[2] = 0;
	this->constraints.resize(3);
	this->scale_x = this->scale_y = this->scale_z = 1.0;
}

/* This is only done for seeding mode. After a single step (ie. a grid cell is evaluated),
we find the faces of that grid cell that has an intersection, and add the grid cells 
adjacent to that face on a queue for later evaluation. */
void Marching::find_cubes_for_seeding()
{
	bool cube_face_has_intersection[6] = { false };
	bool edge_has_intersection[12] = { false };
	xyz next_cube;

	//find the edges with intersection
	for (int i = 0; i < this->poly_data.step_data.edge_list.size(); i++)
		edge_has_intersection[this->poly_data.step_data.edge_list[i]] = true;

	float x_curr = this->poly_data.step_data.corner_coords[0]; //x0 from last step
	float y_curr = this->poly_data.step_data.corner_coords[1]; //y0 from last step
	float z_curr = this->poly_data.step_data.corner_coords[2]; //z0 from last step

	//cout << "another find cube call from (" << x_curr << "," << y_curr << "," << z_curr << ")" << endl;

	//find the faces on the cube that has an intersection
	// by checking the 4 edges of that face. 
	for (int f = 0; f < 6; f++){
		for (int e = 0; e < 4; e++){
			if (edge_has_intersection[cube_face_edge_table[f][e]]){
				cube_face_has_intersection[f] = true;
				break;
			}
		}
	}

	//printf("for current cube %f, %f, %f\n", x_curr, y_curr, z_curr);
	//We keep a set of grid cells that has been evaluated or are already on the queue to be evaluated.
	//Here, for each grid cell adjacent to a face with intersection, if the gric cell is not in
	//  the set, we add it to the set and queue it for evaluation.
	for (int f = 0; f < 6; f++){
		if (!cube_face_has_intersection[f])
			continue;

		//calculate the adjacent cell
		next_cube.x = x_curr + cube_face_normal[f][0] * grid_step_size;
		next_cube.y = y_curr + cube_face_normal[f][1] * grid_step_size;
		next_cube.z = z_curr + cube_face_normal[f][2] * grid_step_size;
		next_cube.idx = poly_data.step_data.step_i;

		//check if it is inside the [-1,1] bound
		if (next_cube.x >= -1 - 0.5*this->grid_step_size && (next_cube.x + 0.5*this->grid_step_size) <= 1 &&
			next_cube.y >= -1 - 0.5*this->grid_step_size && (next_cube.y + 0.5*this->grid_step_size) <= 1 &&
			next_cube.z >= -1 - 0.5*this->grid_step_size && (next_cube.z + 0.5*this->grid_step_size) <= 1)
		{
			//try to put the grid cell onto the set
			bool seed_set_insert_successful = my_seed_set.insert(next_cube).second;
			
			/*printf(" %f %f %f - %x %x %x - %d \n", next_cube.x, next_cube.y, next_cube.z, 
				*(unsigned int*)&next_cube.x, *(unsigned int*)&next_cube.y, *(unsigned int*)&next_cube.z,
				seed_set_insert_successful);*/
			
				//only push this on the queue if it has not already been on the queue
			if (seed_set_insert_successful){
				seed_queue.push_back(next_cube);
			}
		}
	}
}

//calculate and put the starting seed into seed_grid
void Marching::get_starting_seed_grid( xyz* seed_grid)
{   
	float dx = ((seed[0] / scale_x - (-1)) / grid_step_size);
	float dy = ((seed[1] / scale_y - (-1)) / grid_step_size);
	float dz = ((seed[2] / scale_z - (-1)) / grid_step_size);
	seed_grid->x = -1 + floor(dx)*grid_step_size;
	seed_grid->y = -1 + floor(dy)*grid_step_size;
	seed_grid->z = -1 + floor(dz)*grid_step_size;
	//cout <<"get_grid"<< seed_grid.x0 << "  " << seed_grid.y0 << "  " << seed_grid.z0 << endl;
}

void Marching::seed_mode(bool b) {
	this->is_seed_mode = b;
	this->reset_step();
}

void Marching::get_seed(float *x, float *y, float *z) {
	*x = seed[0];
	*y = seed[1];
	*z = seed[2];
}

bool Marching::set_seed(float x, float y, float z)
{
	//check that the seed is within [-1,1] bound
	if ((x <= 1 && x >= -1) && (y >= -1 && y <= 1) && (z >= -1 && z <= 1))
	{
		seed[0] = x; seed[1] = y; seed[2] = z;
		this->reset_step();
		return true;
	}
	else
		return false;
}


bool Marching::set_evaluator(Evaluator* e){
	if (e){
		evaluator = e;
		return true;
	}
	else 
		return false;
}

void Marching::set_surface_constant(float c){
	if (this->surface_constant != c){
		this->surface_constant = c;
		this->reset_step();
	}
}

bool Marching::set_surface_repeat_step_distance(float l){
	if (l <= 0)
		return false;
	this->surface_step = l;
	this->reset_step();
	return true;
}

bool Marching::repeating_surface_mode(bool b){
	if (this->surface_step < 0){
		return this->is_repeating_surface = false;
	}
	this->is_repeating_surface = b;
	this->reset_step();
	return true;
}

bool Marching::set_constraint(int constraint_i, string constraint_lhs, string compare_op, float rhs_value){
	if (constraint_i <0 || constraint_i > 2)
		return false;

	if (this->constraints.size() < (size_t)constraint_i + 1)
		this->constraints.resize(constraint_i + 1);

	//check operation string
	Comp_Op op;
	if (!compare_op.compare("<="))
		op = Comp_Op::LE;
	else if (!compare_op.compare(">="))
		op = Comp_Op::GE;
	else if (!compare_op.compare("<"))
		op = Comp_Op::LT;
	else if (!compare_op.compare(">"))
		op = Comp_Op::GT;
	else
		return false;
	bool set_eq_successful = this->constraints[constraint_i].eval.set_equation(constraint_lhs); 
	if (!set_eq_successful) return false;

	this->constraints[constraint_i].valid = 1;
	this->constraints[constraint_i].lhs = constraint_lhs;
	this->constraints[constraint_i].op = op;
	this->constraints[constraint_i].rhs = rhs_value;
	this->reset_step();
}

bool Marching::use_constraint(int constraint_i, bool use){
	if (constraint_i >= 0 && constraint_i >= this->constraints.size())
		return false;
	this->reset_step();
	return this->constraints[constraint_i].in_use = use;
}

float Marching::evaluate(Evaluator* e, float x, float y, float z){
	if (e) {
		float v = e->evaluate(scale_x * x, scale_y * y, scale_z * z);
		/*if (v==NAN || isinf(v))
		cout << "v:" << v << " isnan"<<(v==NAN)<<"  isinf"<<isinf(v)<< endl;
		*/
		return v;
	}
	else {
		return 0;
	}
}

float Marching::evaluate(float x, float y, float z){
	return this->evaluate(this->evaluator, x, y, z);
}

bool Marching::set_grid_step_size(float v){
	if (v >= 0.001 && v <= .5){
		if (v != grid_step_size){
			this->grid_step_size = v;
			reset_step();
		}
		return true;
	}
	else {
		return false;
	}

}

void Marching::set_scaling_x(float s){
	this->scale_x = s;
	this->reset_step();
}
void Marching::set_scaling_y(float s){
	this->scale_y = s;
	this->reset_step();
}
void Marching::set_scaling_z(float s){
	this->scale_z = s;
	this->reset_step();
}


// return true if the point is within constraints
bool Marching::check_constraints(float x, float y, float z){
	bool within_constraints = true;
	for (size_t i = 0; i < this->constraints.size(); i++){
		if (constraints[i].valid && constraints[i].in_use){
			float lhs = this->evaluate( &constraints[i].eval, x, y, z);
			float rhs = constraints[i].rhs;
			switch (this->constraints[i].op){
			case Comp_Op::GE:
				within_constraints &= lhs >= rhs;
				break;
			case Comp_Op::LE:
				within_constraints &= lhs <= rhs;
				break;
			case Comp_Op::GT:
				within_constraints &= lhs > rhs;
				break;
			case Comp_Op::LT:
				within_constraints &= lhs < rhs;
				break;
			case Comp_Op::NAO:
				printf("Constraint incorrect\n");
			}
		}
	}
	return within_constraints;
}


void Marching::step_by_step_mode(bool mode){
	this->is_step_by_step = mode;
	this->reset_step();
	
}

void Marching::reset_step(){
	this->poly_data.step_data.step_i = -2;
}

void Marching::reset_all_data(){
	this->poly_data.tri_list.clear();
	this->poly_data.vertex_list.clear();
	this->poly_data.step_data.intersect_coord.clear();
	this->poly_data.step_data.tri_vlist.clear();
	this->poly_data.step_data.edge_list.clear();

	this->vertex_set.clear();
	this->my_seed_set.clear();
	while (!seed_queue.empty())
		seed_queue.pop_front();
	reset_step();
}

//Update poly_data by evaluating grid cells, 1 cell at a time if step_by_step_mode = on
bool Marching::recalculate(){

	if (is_seed_mode){
		if (!this->is_step_by_step)	{
			//Evaluating all mesh-connected grid cells starting at the seed
			
			reset_all_data();
			
			// Get the seed grid to start with
			xyz seed_grid;
			get_starting_seed_grid(&seed_grid);
			seed_queue.push_back(seed_grid);
			my_seed_set.insert(seed_grid);

			// Evaluate grid on the queue until the queue is empty. 
			// At each evaluation, connected grid cells are added onto the queue
			while (!seed_queue.empty())	{
				seed_grid = seed_queue.front();
				seed_queue.pop_front();
				calculate_step(seed_grid.x, seed_grid.y, seed_grid.z); 
				this->add_step_to_poly_data();
				find_cubes_for_seeding();
			}
		}
		else { //seed mode with step by step
			xyz seed_grid;
			if (this->poly_data.step_data.step_i == 0) { //last step 
				add_step_to_poly_data();
				this->poly_data.step_data.step_i = -1;
				return true;
			}
			else if (this->poly_data.step_data.step_i == -1) { //finished steps already. no updates
				return false;
			}
			else if (this->poly_data.step_data.step_i == -2) { //first step
				reset_all_data();

				get_starting_seed_grid(&seed_grid);
				seed_grid.idx = 1;
				my_seed_set.insert(seed_grid);
				this->poly_data.step_data.step_i = 1;						
			}
			else { //step i some positive value means we are in the middle of the step process
				if (!seed_queue.empty())
				{
					seed_grid = seed_queue.front();
					seed_queue.pop_front();
					this->poly_data.step_data.step_i++;
				}
				else {
					this->poly_data.step_data.step_i = 0;
				}
				add_step_to_poly_data();
			}
				
			this->calculate_step(seed_grid.x, seed_grid.y, seed_grid.z);
			find_cubes_for_seeding();

		}
	} 
	else { // None-seed mode
		if (!this->is_step_by_step) { //draw all, grid by grid.
			reset_all_data();
			
			float x_0, y_0, z_0;
			float lower_bound = -1.0;
			float upper_bound = 1.0 + 0.5* this->grid_step_size;
			for (z_0 = lower_bound; z_0 <= upper_bound; z_0 += this->grid_step_size) {
				for (y_0 = lower_bound; y_0 <= upper_bound; y_0 += this->grid_step_size) {
					for (x_0 = lower_bound; x_0 <= upper_bound; x_0 += this->grid_step_size) {

						this->calculate_step(x_0, y_0, z_0);
						this->add_step_to_poly_data();
					}
				}
			}
		}

		else { //step by step
			float x_0, y_0, z_0;
			if (this->poly_data.step_data.step_i == 0){ //last step 
				add_step_to_poly_data();
				this->poly_data.step_data.step_i = -1;
				return true;
			}
			if (this->poly_data.step_data.step_i == -1){ //finished steps already. no updates
				return false;
			}
			if (this->poly_data.step_data.step_i == -2){ //first step
				reset_all_data();

				x_0 = y_0 = z_0 = -1;

				this->poly_data.step_data.step_i = 0;
				for (float x0 = -1.0; x0 < 1.0; x0 += this->grid_step_size)
					this->poly_data.step_data.step_i++;
				this->poly_data.step_data.step_i *= this->poly_data.step_data.step_i * this->poly_data.step_data.step_i;
				this->poly_data.step_data.step_i--;
			}
			else { // some steps after first step
				x_0 = this->poly_data.step_data.corner_coords[3]; //x_1 from last step 
				y_0 = this->poly_data.step_data.corner_coords[4]; //y_0 from last step
				z_0 = this->poly_data.step_data.corner_coords[5]; //z_0 from last step
				if (x_0 >= 1.0) {
			        x_0 = -1;
					y_0 += this->grid_step_size;
				}
				if (y_0 >= 1.0) {
					y_0 = -1;
					z_0 += this->grid_step_size;
				}
				this->poly_data.step_data.step_i--;
			}
			
			//cout << this->poly_data.step_data.step_i << endl;;
			
			add_step_to_poly_data();
			this->calculate_step(x_0, y_0, z_0);
		

		}
	}
	//for (int i = 0; i < poly_data.tri_list.size(); i++)
		//std::cout << poly_data.tri_list[i] << " ";
	return true;
}

// Interpolate on a 1d line starting at x_s with value v_s and ending at x_e 
//  with value v_e, find x where the value is this->surface_constant
float Marching::interp(float x_s, float x_e, float v_s, float v_e)
{ 
	float v = ((this->surface_constant - v_s) / (v_e - v_s)) * (x_e - x_s);
	if (isinf(v))
		return  x_s + 0.5*(x_e - x_s);
	if (isnan(v))
		return x_s + 0.5*(x_e - x_s);
	return x_s + v;

}

/* Construct the marching cube surface for a grid cell.
The grid cell's lower x,y,z values of its corners is given in the parameter.
The upper value of its corner is then calculated from the grid size.

The function evaluates the grid cell corners' xyz value, create a look-up code from 
those values depending on whether it is larger or smaller than the surface constant, 
then use the look-up table value to construct the triangles and store it in step_data.
*/
void Marching::calculate_step(float x_0, float y_0, float z_0){
	//get the grid upper boundary
	float x_1 = x_0 + this->grid_step_size;
	float y_1 = y_0 + this->grid_step_size;
	float z_1 = z_0 + this->grid_step_size;

	//cout << x_0 << " " << y_0 << " " << z_0 << " " << x_1 << " " << y_1 << " "<<z_1<<endl;
	
	//clear out step_data
	Step_Data* step = &this->poly_data.step_data;
	step->intersect_coord.clear();
	step->tri_vlist.clear();
	step->edge_list.clear();
	
	//set the x,y,z of the 8 points of the grid cell
	step->corner_coords = { x_0, y_0, z_0, x_1, y_0, z_0, x_1, y_1, z_0, x_0, y_1, z_0, 
		x_0, y_0, z_1, x_1, y_0, z_1, x_1, y_1, z_1, x_0, y_1, z_1 };
	
	//evaluate the values at the 8 corners of the grid cell
	for (int i = 0; i < 8; i++){
		if(!check_constraints(step->corner_coords[3 * i], step->corner_coords[3 * i + 1], step->corner_coords[3 * i + 2]))
			return;
		step->corner_values[i] = this->evaluate(step->corner_coords[3 * i], step->corner_coords[3 * i + 1], step->corner_coords[3 * i + 2]);
	}

	//If the repeating-surface option is in use, change this->surface constant temporarily to calculate correctly.
	//This option is not presented in the UI.
	float surface_const = this->surface_constant;
	if (this->is_repeating_surface){
		float corner_val_min = step->corner_values[0]; float corner_val_max = step->corner_values[0];
		for (int i = 1; i < 8; i++){
			if (corner_val_min > step->corner_values[i]) corner_val_min = step->corner_values[i];
			if (corner_val_max < step->corner_values[i]) corner_val_max = step->corner_values[i];
		}
		float a = (corner_val_max - this->surface_constant) / this->surface_step;
		a = floor(a);
		surface_const = this->surface_constant + this->surface_step * a;
		step->surf_constant = surface_const;
	}
	
	// Calculate the configuration code for look-up on the marching cube table
	int cube_code = 0;
	if (step->corner_values[0] > surface_const) cube_code |= 1;
	if (step->corner_values[1] > surface_const) cube_code |= 2;
	if (step->corner_values[2] > surface_const) cube_code |= 4;
	if (step->corner_values[3] > surface_const) cube_code |= 8;
	if (step->corner_values[4] > surface_const) cube_code |= 16;
	if (step->corner_values[5] > surface_const) cube_code |= 32;
	if (step->corner_values[6] > surface_const) cube_code |= 64;
	if (step->corner_values[7] > surface_const) cube_code |= 128;

	//nothing is drawn if the cube is all positive or all negative.
	if (cube_code == 0 || cube_code == 255) { 
		return;
	}

	/*if (this->is_step_by_step)
		print_step_info(); //for debug
		*/

	// look-up the triangle edge list. 
	int* tri_edge_list = tri_table[cube_code];

	//For ambiguous cases, check and replace with alternative index if needed
	//int flipped_tri_table[16]; //a temporary storage for tri_table entry 
	int alternative_idx = ambiguity_check_and_redirect[cube_code][0];
	if (alternative_idx >= 0){ //ambiguous case if the alt_idx is positive
		// We resolve ambiguity by finding the mid-point of the listed vertices to check, 
		//   and see if the midpoint is consistant with the surface for that look-up entry.
		// By design, the surfaces are formed to assume the midpoint to be positive. 
		//   If this is not the case, then the new look-up index is used instead.

		//with the triangles flipped to account for ambiguity cases

		//list of edges to find midpoint for
		int* vert_to_check_list = ambiguity_check_and_redirect[cube_code]+1;
		float midx = 0, midy = 0, midz = 0;
		for (int i = 0; i < 4; i++){
			int vi = vert_to_check_list[i];
			midx += step->corner_coords[vi * 3];
			midy += step->corner_coords[vi * 3 + 1];
			midz += step->corner_coords[vi * 3 + 2];
		}
		midx /= 4.0; midy /= 4.0; midz /= 4.0;

		//evaluate the value at this midpoint
		float mid_val = this->evaluate(midx, midy, midz);

		// Change the look-up table to the new table
		if (mid_val > surface_const){
			tri_edge_list = tri_table[alternative_idx];
		}
		
	}

	// Calculate intersection coordinates by interpolating linearly the corner values 
	//   along the cell grid edges, and finding where the surface constant lies. 
	//   The intersection coordinates will be the vertices of the triangles that forms 
	//   the marching cube surface.
	this->poly_data.step_data.edge_list.clear();
	unsigned int mapper[12] = { 12 }; //to map edge idx to the idx in intersect_coord
	for (int ei = 0; ei < 12; ei++){
		//For every grid celll edges, see if the sign (positive/negative) of its 2 vertices is different.
		//If it's different, then there is an intersection on that edge.
		int v1 = cube_edge_vertex_table[ei][0]; //vertex 1
		int v2 = cube_edge_vertex_table[ei][1]; //vertex 2
		//the cube code for the 2 vertices
		bool v1_val = (cube_code & two_to_the[v1]); 
		bool v2_val = (cube_code & two_to_the[v2]); 
		
		if (v1_val != v2_val){ //this edge has an intersecting point
			step->edge_list.push_back(ei); //add the edge to a list
				
			//find the intersection point on that edge
			float x_interp = interp(step->corner_coords[v1 * 3], step->corner_coords[v2 * 3], step->corner_values[v1], step->corner_values[v2]);
			float y_interp = interp(step->corner_coords[v1 * 3 + 1], step->corner_coords[v2 * 3 + 1], step->corner_values[v1], step->corner_values[v2]);
			float z_interp = interp(step->corner_coords[v1 * 3 + 2], step->corner_coords[v2 * 3 + 2], step->corner_values[v1], step->corner_values[v2]);

			// for mapping edge idx to the point
			mapper[ei] = step->intersect_coord.size() / 3;

			// storing the intersection coordinate
			step->intersect_coord.push_back(x_interp);
			step->intersect_coord.push_back(y_interp);
			step->intersect_coord.push_back(z_interp);

		}
	}

	//go through tri_table, store the triangle vertex indices 
	for (int i = 0; i < 16; i +=3){
		int v1 = tri_edge_list[i];
		int v2 = tri_edge_list[i+1];
		int v3 = tri_edge_list[i+2];
		if (v1 == -1) break;
		step->tri_vlist.push_back(mapper[v1]);
		step->tri_vlist.push_back(mapper[v2]);
		step->tri_vlist.push_back(mapper[v3]);
	}
}

/* After calculating a grid cell, add the data to Poly_Data, which contains
the surface mesh made from all the evaluated grid cells. */
void Marching::add_step_to_poly_data(){
	Step_Data* step = &this->poly_data.step_data;

	int v_i_list[12];
	for (int i = 0; i < 12; i++)
		v_i_list[i] = -1;

	//add vertices
	for (int i = 0; i < step->intersect_coord.size(); i +=3){
		float x = step->intersect_coord[i];
		float y = step->intersect_coord[i+1];
		float z = step->intersect_coord[i+2];
		if (!isnan(x)){
			v_i_list[i / 3] = add_point(x, y, z);
		}
	}

	//add triangles
	for (int i = 0; i < step->tri_vlist.size(); i+=3){
		int v1 = v_i_list[ step->tri_vlist[i] ];
		int v2 = v_i_list[ step->tri_vlist[i + 1] ];
		int v3 = v_i_list[ step->tri_vlist[i + 2] ];
		this->add_triangle(v1,v2,v3);
	}
}

// add a point to Poly_Data. Return the point index. Will return point index of 
//   an existing points if they are considered the same points.
int Marching::add_point(float xval, float yval, float zval){
	int new_vertex_i = this->poly_data.vertex_list.size() / 3; //index of the new vertex
	//cout << new_vertex_i <<	"  " << xval << "\t" << yval << "\t" << zval  ;
	int v_i_found = vertex_set.insert(xyz(xval, yval, zval, new_vertex_i)).first->idx;
	if (v_i_found == new_vertex_i) // There are no same points in the set
	{
		this->poly_data.vertex_list.push_back(xval);
		this->poly_data.vertex_list.push_back(yval);
		this->poly_data.vertex_list.push_back(zval);
	}
	else{
		//cout << "\t->"<< v_i_found;
	}
	//cout << endl;
	
	return v_i_found;
}

// Add a triangle to Poly_Data
int Marching::add_triangle(int p1, int p2, int p3){
	int new_tri_i = this->poly_data.tri_list.size();

	this->poly_data.tri_list.push_back(p1);
	this->poly_data.tri_list.push_back(p2);
	this->poly_data.tri_list.push_back(p3);

	return new_tri_i;
}

Poly_Data const * Marching::get_poly_data(){
	return &this->poly_data;
}

deque<xyz> const * Marching::get_seed_queue(){
	return &this->seed_queue;
}

//Load a .ply mesh from file to display
bool Marching::load_poly_from_file(){

	reset_all_data();
	
	FILE *fp;
	OPENFILENAME OpenFilename;
	TCHAR	szFile[MAX_PATH] = "mesh.ply";
	TCHAR	szTitle[MAX_PATH] = "points";
	static TCHAR szFilter[] = "file type\0*.*\0file type1\0*.*\0";
	OpenFilename.lStructSize = sizeof(OPENFILENAME);
	OpenFilename.hwndOwner = NULL;
	OpenFilename.hInstance = NULL;
	OpenFilename.lpstrFilter = szFilter;
	OpenFilename.lpstrCustomFilter = NULL;
	OpenFilename.nMaxCustFilter = 0;
	OpenFilename.nFilterIndex = 0;
	OpenFilename.lpstrFile = szFile;
	OpenFilename.nMaxFile = MAX_PATH;
	OpenFilename.lpstrFileTitle = szTitle;
	OpenFilename.nMaxFileTitle = MAX_PATH;
	OpenFilename.lpstrInitialDir = ".";
	OpenFilename.lpstrTitle = "Load Mesh";
	OpenFilename.Flags = OFN_EXPLORER;
	OpenFilename.nFileOffset = 0;
	OpenFilename.nFileExtension = 0;
	OpenFilename.lpstrDefExt = TEXT("ply");
	OpenFilename.lCustData = 0L;
	OpenFilename.lpfnHook = NULL;
	OpenFilename.lpTemplateName = NULL;
	if (!GetOpenFileName(&OpenFilename))
		return false;
	errno_t err = fopen_s(&fp, OpenFilename.lpstrFile, "r");
	if (NULL == fp)
		return  false;
	int num_of_points = 0; int num_of_triangles = 0;
	float x = 0, y = 0, z = 0;
	char char_arr[256];
	string str;
	fgets(char_arr, _countof(char_arr), fp);
	str = string(char_arr);
	if (str.find("ply") == -1){
		fclose(fp);
		return false;
	}
	fgets(char_arr, _countof(char_arr), fp);
	str = string(char_arr);
	if (str.find("format ascii 1.0") == -1){
		fclose(fp);
		return false;
	}
	fgets(char_arr, _countof(char_arr), fp);
	str = string(char_arr);
	int counter = 2;
	while (str.find("end_header") == -1)
	{
		counter++;
		if (counter >= 10){
			fclose(fp);
			return false;
		}
		if (str.find("element vertex") != -1)
		{
			num_of_points = stoi(str.substr(15, str.length() - 15));
			//cout << "num_of_points" << num_of_points << endl;
		}
		if (str.find("element face") != -1)
		{
			num_of_triangles = stoi(str.substr(13, str.length() - 13));
			//cout << "number of triangles" << num_of_triangles << endl;
		}
		if (str == "") break;
		fgets(char_arr, _countof(char_arr), fp);
		str = string(char_arr);
	}

	for (int i = 0; i < num_of_points; i++){
		if (fscanf_s(fp, "%f", &x) == EOF ||
			(fscanf_s(fp, "%f", &y) == EOF) ||
			(fscanf_s(fp, "%f", &z) == EOF))
		{
			fclose(fp);
			return false;
		}
		//cout << "x=" << x << "y=" << y << "z=" << z << endl;
		poly_data.vertex_list.push_back(x);
		poly_data.vertex_list.push_back(y);
		poly_data.vertex_list.push_back(z);
	}
	unsigned int num = 0, x0, y0, z0;

	for (int i = 0; i < num_of_triangles; i++)
	{
		if (fscanf_s(fp, "%u %u %u %u", &num, &x0, &y0, &z0) == EOF) return false;
		if (num != 3){
			fclose(fp);
			return false;
		}

		//cout << "x0=" << x0 << "y0=" << y0 << "z0=" << z0 << endl;
		poly_data.tri_list.push_back(x0);
		poly_data.tri_list.push_back(y0);
		poly_data.tri_list.push_back(z0);

	}
	fclose(fp);
	return true;
}

//Save the generated mesh to a .ply file
bool Marching::save_poly_to_file(){
	FILE *fp;
	OPENFILENAME OpenFilename;
	TCHAR	szFile[MAX_PATH] = "mesh.ply";
	TCHAR	szTitle[MAX_PATH] = "points";
	static TCHAR szFilter[] = "file type\0*.*\0file type1\0*.*\0";
	OpenFilename.lStructSize = sizeof (OPENFILENAME);
	OpenFilename.hwndOwner = NULL;
	OpenFilename.hInstance = NULL;
	OpenFilename.lpstrFilter = szFilter;
	OpenFilename.lpstrCustomFilter = NULL;
	OpenFilename.nMaxCustFilter = 0;
	OpenFilename.nFilterIndex = 0;
	OpenFilename.lpstrFile = szFile;
	OpenFilename.nMaxFile = MAX_PATH;
	OpenFilename.lpstrFileTitle = szTitle;
	OpenFilename.nMaxFileTitle = MAX_PATH;
	OpenFilename.lpstrInitialDir = ".";
	OpenFilename.lpstrTitle = "Save Mesh";
	OpenFilename.Flags = OFN_EXPLORER;
	OpenFilename.nFileOffset = 0;
	OpenFilename.nFileExtension = 0;
	OpenFilename.lpstrDefExt = TEXT("poly");
	OpenFilename.lCustData = 0L;
	OpenFilename.lpfnHook = NULL;
	OpenFilename.lpTemplateName = NULL;

	if (this->poly_data.vertex_list.empty())
		return false;

	if (GetSaveFileName(&OpenFilename) == false)
		return false;

	errno_t err = fopen_s(&fp, OpenFilename.lpstrFile, "w");

	if (NULL == fp)
		return false;

	int num_of_points = 0; int num_of_triangles = 0;
	float x = 0, y = 0, z = 0;
	num_of_points = ((this->poly_data.vertex_list.size()) / 3);

	//cout << num_of_points;
	char buffer[100];
	num_of_triangles = ((poly_data.tri_list.size()) / 3);
	unsigned int x0 = 0, y0 = 0, z0 = 0;
	//adding the header of the poly file 
	sprintf_s(buffer, "%s", "ply\nformat ascii 1.0\n");
	fprintf_s(fp, "%s", buffer);
	//cout << "buffer=" << buffer << endl;
	sprintf_s(buffer, "element vertex %d\n", num_of_points);
	fprintf_s(fp, "%s", buffer);
	sprintf_s(buffer, "%s", "property float x\nproperty float y\nproperty float z\n");
	fprintf_s(fp, "%s", buffer);
	sprintf_s(buffer, "element face %d \n", num_of_triangles);
	fprintf_s(fp, "%s", buffer);
	sprintf_s(buffer, "%s", "property list uchar int vertex_indices\nend_header\n");
	fprintf_s(fp, "%s", buffer);
	for (int i = 0; i < num_of_points; i++)
	{
		x = poly_data.vertex_list[i * 3];
		y = poly_data.vertex_list[i * 3 + 1];
		z = poly_data.vertex_list[i * 3 + 2];
		sprintf_s(buffer, "%f %f %f\n", x, y, z);
		fprintf_s(fp, "%s", buffer);
	}


	for (int i = 0; i < num_of_triangles; i++)
	{
		x0 = poly_data.tri_list.at(i * 3);
		y0 = poly_data.tri_list.at(i * 3 + 1);
		z0 = poly_data.tri_list.at(i * 3 + 2);
		sprintf_s(buffer, "%u %u %u %u\n", 3, x0, y0, z0);
		fprintf_s(fp, "%s", buffer);

	}
	fclose(fp);
	return true;

}


Comp_Op parse_comp_op(string op){
	if (op.compare(">"))
		return Comp_Op::GT;
	if (op.compare("<"))
		return Comp_Op::LT;
	if (op.compare(">="))
		return Comp_Op::GE;
	if (op.compare("<="))
		return Comp_Op::LE;

	return Comp_Op::NAO;
}