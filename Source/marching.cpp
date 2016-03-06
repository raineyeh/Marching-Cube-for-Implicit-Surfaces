#include "marching.h"
#include "marching_lookup.h"
#include <iostream>

float radius = .25;
Marching::Marching(void){
	this->grid_step_size = (float)0.2;
	this->evaluator = NULL;
	this->poly_data.step_data.corner_coords.resize(12);
	this->poly_data.step_data.corner_values.resize(4);
	this->is_step_by_step = false;
}

bool Marching::set_evaluator(Evaluator* e){
	if (e){
		evaluator = e;
		return true;
	}
	else 
		return false;
}

float Marching::evaluate(float x, float y, float z){
	if (this->evaluator) {
		//return abs(x + y) - radius;
		return this->evaluator->evaluate(x, y, z);
	}
	else {
		return 0;
	}
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


void Marching::step_by_step_mode(bool mode){
	this->is_step_by_step = mode;
	
}

void Marching::reset_step(){
	this->poly_data.step_data.step_i = -2;
}

bool Marching::recalculate(){
	if (!this->is_step_by_step){
		radius -= .01;

		this->poly_data.step_data.step_i = -2;

		this->poly_data.tri_list.clear();
		this->poly_data.vertex_list.clear();

		float x_0, x_1, y_0, y_1;
		for (y_0 = -1.0; y_0 < 1.0; y_0 += this->grid_step_size){
			y_1 = y_0 + this->grid_step_size;
			for (x_0 = -1.0; x_0 < 1.0 ; x_0 += this->grid_step_size){
				x_1 = x_0 + this->grid_step_size;
		
				this->do_square(x_0, x_1, y_0, y_1);
				add_step_to_poly_data();
			}
		}
	}
	else{
		float x_0, x_1, y_0, y_1;
		if (this->poly_data.step_data.step_i == 0){ //last step 
			add_step_to_poly_data();
			this->poly_data.step_data.step_i = -1;
			return true;
		}
		if (this->poly_data.step_data.step_i == -1){ //finished steps already. no updates
			return false;
		}
		if (this->poly_data.step_data.step_i == -2){ //first step
			x_0 = -1; y_0 = -1;

			//this->poly_data.step_data.step_i = (int)((2.0 + this->grid_step_size) / this->grid_step_size); //not accurate enough
			this->poly_data.step_data.step_i = 0;
			for (float x0 = -1.0 ; x0 < 1.0; x0 += this->grid_step_size)
				this->poly_data.step_data.step_i++;
			//cout << "one side: " << this->poly_data.step_data.step_i << endl;
			this->poly_data.step_data.step_i *= this->poly_data.step_data.step_i;

		}
		else{
			x_0 = this->poly_data.step_data.corner_coords[3]; //x_1 from last step 
			y_0 = this->poly_data.step_data.corner_coords[4]; //x_2 from last step
			if (x_0 >= 1.0){
				x_0 = -1;
				y_0 += this->grid_step_size;
			}
			this->poly_data.step_data.step_i--;
		}

		x_1 = x_0 + this->grid_step_size;
		y_1 = y_0 + this->grid_step_size;

		//cout << this->poly_data.step_data.step_i << " :";
		if (this->poly_data.step_data.step_i > 0){
			add_step_to_poly_data();
			this->do_square(x_0, x_1, y_0, y_1);
		}
		
	}

	//for (int i = 0; i < poly_data.tri_list.size(); i++)
		//std::cout << poly_data.tri_list[i] << " ";

	return true;
}

float interp(float x_s, float x_e, float v_s, float v_e){ //assumes interp zero
	return x_s + (-v_s / (v_e - v_s)) * (x_e - x_s);

}

void Marching::do_square(float x_0, float x_1, float y_0, float y_1){
	//cout << x_0 << " " << y_0 << " " << x_1 << " " << y_1 << endl;
	Step_Data* step = &this->poly_data.step_data;
	step->corner_coords = { x_0, y_0, 0, x_1, y_0, 0, x_1, y_1, 0, x_0, y_1, 0 };
	step->intersect_coord.clear();
	//float corner_coords[8] = { x_0, y_0, x_1, y_0, x_1, y_1, x_0, y_1 };
	//float corner_values[4];
	//float intersect_coord[8] = { 0 };
	//int edge_pt_count = 0;

	for (int i = 0; i < 4; i++)
		step->corner_values[i] = this->evaluate(step->corner_coords[3 * i], step->corner_coords[3 * i + 1], 0);

	int square_index = 0;
	if (step->corner_values[0] > 0) square_index |= 1;
	if (step->corner_values[1] > 0) square_index |= 2;
	if (step->corner_values[2] > 0) square_index |= 4;
	if (step->corner_values[3] > 0) square_index |= 8;

	int* lines_edge = line_table[square_index];
	int alternative_idx = ambiguous_line_table_redirect[square_index];
	if (alternative_idx != -1){ //ambiguous case
		float midx = 0, midy = 0;
		for (int i = 0; i < 4; i++){
			midx += step->corner_coords[i * 3];
			midy += step->corner_coords[i* 3+1];
		}
		midx /= 4.0; midy /= 4.0;
		float mid_val = this->evaluate(midx, midy, 0);
		if (mid_val < 0)
			lines_edge = line_table[alternative_idx];
	}
	for (int i = 0; i < 4; i ++){
		int line_pt = lines_edge[i];
		if (line_pt == -1) break;
		int vi1 = 3*line_pt;
		int vi2 = 3*((line_pt + 1) % 4);
		float x_interp = interp(step->corner_coords[vi1], step->corner_coords[vi2], step->corner_values[line_pt], step->corner_values[(line_pt + 1) % 4]);
		float y_interp = interp(step->corner_coords[vi1 + 1], step->corner_coords[vi2 + 1], step->corner_values[line_pt], step->corner_values[(line_pt + 1) % 4]);
		step->intersect_coord.push_back(x_interp);
		step->intersect_coord.push_back(y_interp);
		step->intersect_coord.push_back(0); //z
		//step->intersect_coord[2 * i] = x_interp;
		//step->intersect_coord[2 * i + 1] = y_interp;
	}

	//add_step_to_poly_data();
	
	/*for (int i = 0; i < 4; i+=2){
		int line_pt = lines_edge[i];
		int line_pt2 = lines_edge[i+1];
		if (line_pt == -1) break;
		int pi1 = add_point(step->intersect_coord[3 * i], step->intersect_coord[3 * i + 1], 0);
		int pi2 = add_point(step->intersect_coord[3 * (i+1)], step->intersect_coord[3 * (i+1) + 1], 0);
		this->add_line(pi1, pi2);

	}*/

}

void Marching::add_step_to_poly_data(){
	Step_Data* step = &this->poly_data.step_data;
	for (int i = 0; i < step->intersect_coord.size()/3; i +=2){
		int pi1 = add_point(step->intersect_coord[3 * i], step->intersect_coord[3 * i + 1], 0);
		int pi2 = add_point(step->intersect_coord[3 * (i + 1)], step->intersect_coord[3 * (i + 1) + 1], 0);
		this->add_line(pi1, pi2);
	}
	
}

int Marching::add_point(float xval, float yval, float zval){
	int new_vertex_i = this->poly_data.vertex_list.size() / 3;
	this->poly_data.vertex_list.push_back(xval);
	this->poly_data.vertex_list.push_back(yval);
	this->poly_data.vertex_list.push_back(zval);
	
	//std::cout << xval << "\t" << yval << "\t" << zval << ";"<< std::endl;
	
	return new_vertex_i;
}

int Marching::add_line(float x0, float y0, float x1, float y1){
	
	int new_vertex_i1 = add_point(x0, y0, 0);
	int new_vertex_i2 = add_point(x1, y1, 0);
	return add_line(new_vertex_i1, new_vertex_i2);

	

}
int Marching::add_line(int pi1, int pi2){

	int new_line_i = this->poly_data.tri_list.size();
	
	this->poly_data.tri_list.push_back(pi1);
	this->poly_data.tri_list.push_back(pi2);
	//this->poly_data.tri_list.push_back(pi2);

	return new_line_i;

}


Poly_Data const * Marching::get_poly_data(){
	
	/*this->dirty = true;
	poly_data.tri_list.empty();
	poly_data.vertex_list.empty();

	poly_data.vertex_list.resize(9);
	poly_data.vertex_list[0] = .0;
	poly_data.vertex_list[1] = .0;
	poly_data.vertex_list[2] = .0;
	poly_data.vertex_list[3] = -1.0;
	poly_data.vertex_list[4] = -.0;
	poly_data.vertex_list[5] = .0;
	poly_data.vertex_list[6] = .0;
	poly_data.vertex_list[7] = 1.0;
	poly_data.vertex_list[8] = .0;


	poly_data.tri_list.resize(6);	
	poly_data.tri_list[0] = 0;
	poly_data.tri_list[1] = 1;
	poly_data.tri_list[2] = 1;
	poly_data.tri_list[3] = 2;
	poly_data.tri_list[4] = 2;
	poly_data.tri_list[5] = 0;*/

	return &this->poly_data;
}
