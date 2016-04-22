#include "marching.h"
#include "marching_lookup.h"

#include <windows.h>
#include <iostream>
#include <math.h>


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

void Marching::find_cubes_for_seeding()
{
	bool cube_face_has_intersection[6] = { false };
	bool edge_has_intersection[12] = { false };
	xyz next_cube;

	for (int i = 0; i < this->poly_data.step_data.edge_list.size(); i++)
		edge_has_intersection[this->poly_data.step_data.edge_list[i]] = true;

	float x_curr = this->poly_data.step_data.corner_coords[0]; //x0 from last step
	float y_curr = this->poly_data.step_data.corner_coords[1]; //y0 from last step
	float z_curr = this->poly_data.step_data.corner_coords[2]; //z0 from last step

	//cout << "another find cube call from (" << x_curr << "," << y_curr << "," << z_curr << ")" << endl;

	//for each face of the cube, check if an edge has an intersection
	for (int f = 0; f < 6; f++){
		for (int e = 0; e < 4; e++){
			if (edge_has_intersection[cube_face_edge_table[f][e]]){
				cube_face_has_intersection[f] = true;
				break;
			}
		}
	}

	//printf("for current cube %f, %f, %f\n", x_curr, y_curr, z_curr);
	//for each face of the cube, if there is an intersection, try to push the adjacent cube into the set
	for (int f = 0; f < 6; f++){
		if (!cube_face_has_intersection[f])
			continue;

		next_cube.x = x_curr + cube_face_normal[f][0] * grid_step_size;
		next_cube.y = y_curr + cube_face_normal[f][1] * grid_step_size;
		next_cube.z = z_curr + cube_face_normal[f][2] * grid_step_size;
		next_cube.idx = poly_data.step_data.step_i;

		if (next_cube.x >= -1 - 0.5*this->grid_step_size && (next_cube.x + 0.5*this->grid_step_size) <= 1 &&
			next_cube.y >= -1 - 0.5*this->grid_step_size && (next_cube.y + 0.5*this->grid_step_size) <= 1 &&
			next_cube.z >= -1 - 0.5*this->grid_step_size && (next_cube.z + 0.5*this->grid_step_size) <= 1)
		{
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


}//end of procedure 

void Marching::get_starting_seed_grid( xyz* seed_grid)
{   
	float dx = ((seed[0]/scale_x - (-1)) / grid_step_size);
	//seed_grid.x = -1 + floor(dx)*grid_step_size;
	seed_grid->x = -1 + floor(dx)*grid_step_size;
	float dy = ((seed[1] /scale_y - (-1)) / grid_step_size);
	//seed_grid.y = -1+floor(dy)*grid_step_size;
	seed_grid->y = -1 + floor(dy)*grid_step_size;
	float dz = ((seed[2]/ scale_z - (-1)) / grid_step_size);
	//seed_grid.z = -1+floor(dz)*grid_step_size;
	seed_grid->z = -1 + floor(dz)*grid_step_size;
	//cout <<"get_grid"<< seed_grid.x0 << "  " << seed_grid.y0 << "  " << seed_grid.z0 << endl;
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

	this->surface_constant = c;
}

bool Marching::set_surface_repeat_step_distance(float l){
	if (l <= 0)
		return false;
	this->surface_step = l;
	return true;
}

void Marching::seed_mode(bool seed){   
	this->is_seed_mode = seed; 
	
}

bool Marching::set_seed(float x, float y, float z)
{    
	if ((x <= 1 && x >= -1) && (y >= -1 && y <= 1) && (z >= -1 && z <= 1))
	{
		seed[0] = x; seed[1] = y; seed[2] = z;
		return true;
	}
	else
		return false; 
}

bool Marching::repeating_surface_mode(bool b){
	if (this->surface_step < 0){
		return this->is_repeating_surface = false;
	}
	this->is_repeating_surface = b;
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
}

bool Marching::use_constraint(int constraint_i, bool use){
	if (constraint_i >= 0 && constraint_i >= this->constraints.size())
		return false;
	return this->constraints[constraint_i].in_use = use;
}

float Marching::evaluate(Evaluator* e, float x, float y, float z){
	if (e) {
		//return x*x + y*z ;
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
}
void Marching::set_scaling_y(float s){
	this->scale_y = s;
}
void Marching::set_scaling_z(float s){
	this->scale_z = s;
}



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
}

void Marching::reset_step(){
	this->poly_data.step_data.step_i = -2;
}

void Marching::reset_all_data(){
	this->poly_data.tri_list.clear();
	this->poly_data.vertex_list.clear();
	this->poly_data.step_data.intersect_coord.clear();
	this->poly_data.step_data.tri_vlist.clear();

	this->vertex_set.clear();
	this->my_seed_set.clear();
	while (!seed_queue.empty())
		seed_queue.pop_front();
	reset_step();
}

bool Marching::recalculate(){

	if (is_seed_mode){
		xyz seed_grid;
		if (!this->is_step_by_step)
		{//find intersections of the seed grid 
		//call the grids that have common edges with the intersection 

			reset_all_data();

			get_starting_seed_grid(&seed_grid);

			//cout << seed_grid.x0 << "  " << seed_grid.x1 << "   " << seed_grid.y0 << "   " << seed_grid.y1 << "    " << seed_grid.z0 << "    " <<
				//seed_grid.z1 << endl;
			//cout << "the first do square" << endl;
			calculate_step(seed_grid.x, seed_grid.y, seed_grid.z);
			//cout << "the first find cube" << endl;
			this->add_step_to_poly_data();
			
			my_seed_set.insert(seed_grid);
			find_cubes_for_seeding();

			while (!seed_queue.empty())	{
				seed_grid = seed_queue.front();
				seed_queue.pop_front();
				calculate_step(seed_grid.x, seed_grid.y, seed_grid.z); 
				this->add_step_to_poly_data();
				find_cubes_for_seeding();
				//this->poly_data.step_data.edge_list.clear();

			}

		}//end of if seed mode not step by step
		else { //seed mode with step by step
			xyz seed_grid;
			if (this->is_step_by_step) {
				//xyz x;
				if (this->poly_data.step_data.step_i == 0) { //last step 
					add_step_to_poly_data();
					this->poly_data.step_data.step_i = -1;
					return true;
				}
				if (this->poly_data.step_data.step_i == -1) { //finished steps already. no updates
					return false;
				}
				if (this->poly_data.step_data.step_i == -2) { //first step
					reset_all_data();

					get_starting_seed_grid(&seed_grid);
					seed_grid.idx = 1;
					//seed_queue.push_back(seed_grid);
					my_seed_set.insert(seed_grid);

					this->poly_data.step_data.step_i = 1;
									
				}//end first step 
				else//step i some positive value means we are in the middle of the step process
				{
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
	}

	else { //None-seed mode
		if (!this->is_step_by_step) {
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

				//this->poly_data.step_data.step_i = (int)((2.0 + this->grid_step_size) / this->grid_step_size); //not accurate enough
				this->poly_data.step_data.step_i = 0;
				for (float x0 = -1.0; x0 < 1.0; x0 += this->grid_step_size)
					this->poly_data.step_data.step_i++;
				//cout << "one side: " << this->poly_data.step_data.step_i << endl;
				this->poly_data.step_data.step_i *= this->poly_data.step_data.step_i * this->poly_data.step_data.step_i;
				this->poly_data.step_data.step_i--;
			}
			else {
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

float Marching::interp(float x_s, float x_e, float v_s, float v_e)
{ //interpolate to this->surface_constant
	float v = ((this->surface_constant - v_s) / (v_e - v_s)) * (x_e - x_s);
	if (isinf(v))
		return  x_s + 0.5*(x_e - x_s);
	if (isnan(v))
		return x_s + 0.5*(x_e - x_s);
	return x_s + v;

}

void Marching::calculate_step(float x_0, float y_0, float z_0){
	float x_1 = x_0 + this->grid_step_size;
	float y_1 = y_0 + this->grid_step_size;
	float z_1 = z_0 + this->grid_step_size;

	//cout << x_0 << " " << y_0 << " " << z_0 << " " << x_1 << " " << y_1 << " "<<z_1<<endl;
	
	Step_Data* step = &this->poly_data.step_data;
	step->intersect_coord.clear();
	step->tri_vlist.clear();

	step->corner_coords = { x_0, y_0, z_0, x_1, y_0, z_0, x_1, y_1, z_0, x_0, y_1, z_0, 
		x_0, y_0, z_1, x_1, y_0, z_1, x_1, y_1, z_1, x_0, y_1, z_1 };
	
	//calculate the corner values
	for (int i = 0; i < 8; i++){
		if(!check_constraints(step->corner_coords[3 * i], step->corner_coords[3 * i + 1], step->corner_coords[3 * i + 2]))
			return;
		step->corner_values[i] = this->evaluate(step->corner_coords[3 * i], step->corner_coords[3 * i + 1], step->corner_coords[3 * i + 2]);
	}

	//in the case that this->surface_step is set a positive value, change this->surface constant temporarily to calculate correctly.
	float orig_surface_constant = this->surface_constant;
	if (this->is_repeating_surface){
		float corner_val_min = step->corner_values[0]; float corner_val_max = step->corner_values[0];
		for (int i = 1; i < 8; i++){
			if (corner_val_min > step->corner_values[i]) corner_val_min = step->corner_values[i];
			if (corner_val_max < step->corner_values[i]) corner_val_max = step->corner_values[i];
		}
		float a = (corner_val_max - this->surface_constant) / this->surface_step;
		a = floor(a);
		this->surface_constant += this->surface_step * a;
		step->surf_constant = this->surface_constant;
	}
	
	int cube_code = 0;
	if (step->corner_values[0] > this->surface_constant) cube_code |= 1;
	if (step->corner_values[1] > this->surface_constant) cube_code |= 2;
	if (step->corner_values[2] > this->surface_constant) cube_code |= 4;
	if (step->corner_values[3] > this->surface_constant) cube_code |= 8;
	if (step->corner_values[4] > this->surface_constant) cube_code |= 16;
	if (step->corner_values[5] > this->surface_constant) cube_code |= 32;
	if (step->corner_values[6] > this->surface_constant) cube_code |= 64;
	if (step->corner_values[7] > this->surface_constant) cube_code |= 128;

	if (cube_code == 0 || cube_code == 255) {
		this->surface_constant = orig_surface_constant;
		return;
	}

	/*if (this->is_step_by_step)
		print_step_info();
		*/

	int* tri_edge_list = tri_table[cube_code];

	//For ambiguous cases, check and replace with alternative index if needed

	int alternative_idx = ambiguity_check_and_redirect[cube_code][0];
	int flipped_tri_table[16]; //a temporary storage for tri_table entry 
	//with the triangles flipped to account for ambiguity cases
	if (alternative_idx >= 0){ //ambiguous case
		//cout << "ambiguous case!"<< endl;
		
		int* edge_to_check_list = ambiguity_check_and_redirect[cube_code]+1;
		
		float midx = 0, midy = 0, midz = 0;
		for (int i = 0; i < 4; i++){
			int vi = edge_to_check_list[i];
			midx += step->corner_coords[vi * 3];
			midy += step->corner_coords[vi * 3 + 1];
			midz += step->corner_coords[vi * 3 + 2];
		}
		midx /= 4.0; midy /= 4.0; midz /= 4.0;
		float mid_val = this->evaluate(midx, midy, midz);
		if (mid_val > this->surface_constant){
			tri_edge_list = tri_table[alternative_idx];
			for (int i = 0; i < 5; i++){
				flipped_tri_table[i * 3] = tri_edge_list[i * 3];
				flipped_tri_table[i * 3+1] = tri_edge_list[i * 3+2];
				flipped_tri_table[i * 3+2] = tri_edge_list[i * 3+1];
			}
			flipped_tri_table[15] = -1; //last entry
			tri_edge_list = flipped_tri_table;
		}
		
	}

	// calculate intersection coordinates
	this->poly_data.step_data.edge_list.clear();
	step->intersect_coord.resize(36, NAN);
	for (int ei = 0; ei < 12; ei++){
		int v1 = cube_edge_vertex_table[ei][0];
		int v2 = cube_edge_vertex_table[ei][1];
		bool v1_val = (cube_code & two_to_the[v1]) != 0;
		bool v2_val = (cube_code & two_to_the[v2]) != 0;
		
		if (v1_val != v2_val){ //this edge has an intersecting point
			step->edge_list.push_back(ei);
				
			float x_interp = interp(step->corner_coords[v1 * 3], step->corner_coords[v2 * 3], step->corner_values[v1], step->corner_values[v2]);
			float y_interp = interp(step->corner_coords[v1 * 3 + 1], step->corner_coords[v2 * 3 + 1], step->corner_values[v1], step->corner_values[v2]);
			float z_interp = interp(step->corner_coords[v1 * 3 + 2], step->corner_coords[v2 * 3 + 2], step->corner_values[v1], step->corner_values[v2]);

			step->intersect_coord[ei * 3] = x_interp;
			step->intersect_coord[ei * 3 + 1] = y_interp;
			step->intersect_coord[ei * 3 + 2] = z_interp;
		}
		
	}

	//go through tri_table, add triangle vertices
	for (int i = 0; i < 16; i +=3){
		int v1 = tri_edge_list[i];
		int v2 = tri_edge_list[i+1];
		int v3 = tri_edge_list[i+2];
		if (v1 == -1) break;
		step->tri_vlist.push_back(v1);
		step->tri_vlist.push_back(v2);
		step->tri_vlist.push_back(v3);
	}

	//get rid of the NaN points
	int pi = 0;
	for (int i = 0; i < step->intersect_coord.size()/3; i++){
		if (!isnan(step->intersect_coord[i * 3])){
			step->intersect_coord[pi * 3] = step->intersect_coord[i * 3];
			step->intersect_coord[pi * 3+1] = step->intersect_coord[i * 3+1];
			step->intersect_coord[pi * 3+2] = step->intersect_coord[i * 3+2];
			//replace
			for (int j = 0; j < step->tri_vlist.size(); j++){
				if (step->tri_vlist[j] == i)
					step->tri_vlist[j] = pi;
			}
			pi++;
		}

	}
	//cout << "pi:" << pi << endl;
	step->intersect_coord.resize(pi*3);

	//reset the surface constant
	this->surface_constant = orig_surface_constant;
	
}

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

int Marching::add_point(float xval, float yval, float zval){
	int new_vertex_i = this->poly_data.vertex_list.size() / 3;
	//cout << new_vertex_i <<	"  " << xval << "\t" << yval << "\t" << zval  ;
	int v_i_found = vertex_set.insert(xyz(xval, yval, zval, new_vertex_i)).first->idx;
	if (v_i_found == new_vertex_i)
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

bool Marching::load_poly_from_file(){
	
	reset_all_data();
	
	FILE *fp;
	OPENFILENAME OpenFilename;
	TCHAR	szFile[MAX_PATH] = "mesh.poly";
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
	OpenFilename.lpstrInitialDir = NULL;
	OpenFilename.lpstrTitle = "Open";
	OpenFilename.Flags = OFN_EXPLORER;
	OpenFilename.nFileOffset = 0;
	OpenFilename.nFileExtension = 0;
	OpenFilename.lpstrDefExt = TEXT("poly");
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

	fscanf_s(fp, "%d", &num_of_points);
	//cout << "num_of_points" << num_of_points << endl;
	for (int i = 0; i <num_of_points; i++){
		fscanf_s(fp, "%f", &x);
		fscanf_s(fp, "%f", &y);
		fscanf_s(fp, "%f", &z);
		//cout << "x=" << x << "y=" << y << "z=" << z << endl;
		poly_data.vertex_list.push_back(x);
		poly_data.vertex_list.push_back(y);
		poly_data.vertex_list.push_back(z);
	}
	unsigned int x0, y0, z0;
	fscanf_s(fp, "%d", &num_of_triangles);
	//cout << "num_of_triangles" << num_of_triangles << endl;
	for (int i = 0; i <num_of_triangles; i++)
	{
		fscanf_s(fp, "%u %u %u", &x0, &y0, &z0);
		//cout << "x0=" << x0 << "y0=" << y0 << "z0=" << z0 << endl;
		poly_data.tri_list.push_back(x0);
		poly_data.tri_list.push_back(y0);
		poly_data.tri_list.push_back(z0);
	}
	fclose(fp);
	return true;
}

bool Marching::save_poly_to_file(){
	FILE *fp;
	OPENFILENAME OpenFilename;
	TCHAR	szFile[MAX_PATH] = "output.poly";
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
	OpenFilename.lpstrInitialDir = NULL;
	OpenFilename.lpstrTitle = "Save data";
	OpenFilename.Flags = OFN_EXPLORER;
	OpenFilename.nFileOffset = 0;
	OpenFilename.nFileExtension = 0;
	OpenFilename.lpstrDefExt = TEXT("poly");
	OpenFilename.lCustData = 0L;
	OpenFilename.lpfnHook = NULL;
	OpenFilename.lpTemplateName = NULL;

	if ( this->poly_data.vertex_list.empty()) 
		return false;

	if (GetOpenFileName(&OpenFilename) == false) 
		return false;

	errno_t err = fopen_s(&fp, OpenFilename.lpstrFile, "w");

	if (NULL == fp) 
		return false;

	int num_of_points = 0; int num_of_triangles = 0;
	float x = 0, y = 0, z = 0;
	num_of_points = ((this->poly_data.vertex_list.size()) / 3);
	//cout << num_of_points;
	char buffer[50];
	sprintf_s(buffer, "%d", num_of_points);
	fprintf_s(fp, "%s", buffer);
	fprintf_s(fp, "\n");
	for (int i = 0; i < num_of_points; i++)
	{
		x = poly_data.vertex_list[i * 3];
		y = poly_data.vertex_list[i * 3 + 1];
		z = poly_data.vertex_list[i * 3 + 2];
		sprintf_s(buffer, "%f %f %f\n", x, y, z);
		fprintf_s(fp, "%s", buffer);
	}
	num_of_triangles = ((poly_data.tri_list.size()) / 3);
	unsigned int x0 = 0, y0 = 0, z0 = 0;
	sprintf_s(buffer, "%d", num_of_triangles);
	fprintf_s(fp, "%s", buffer);
	fprintf_s(fp, "\n");
	for (int i = 0; i < num_of_triangles; i++)
	{
		x0 = poly_data.tri_list.at(i * 3);
		y0 = poly_data.tri_list.at(i * 3 + 1);
		z0 = poly_data.tri_list.at(i * 3 + 2);
		sprintf_s(buffer, "%u %u %u\n", x0, y0, z0);
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