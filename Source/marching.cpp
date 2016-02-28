#include "marching.h"
#include "marching_lookup.h"
#include <iostream>


Marching::Marching(void){
	this->grid_step_size = .2;
	this->dirty = true;
	this->evaluator = NULL;
	
}

bool Marching::set_evaluator(Evaluator* e){
	if (e){
		evaluator = e;
		dirty = true;
		return true;
	}
	else 
		return false;
}

float Marching::evaluate(float x, float y, float z){
	if (this->evaluator) {
		return this->evaluator->evaluate(x, y, z);
	}
	else {
		return 0;
	}
}

bool Marching::set_grid_step_size(float v){
	if (v > 0 && v <= .5){
		this->grid_step_size = v;
		dirty = true;
		return true;
	}
	else {
		return false;
	}

}

bool Marching::recalculate(){
	if (!this->dirty) return true;


	float x_0, x_1, y_0, y_1;
	for (x_0 = -1.0; x_0 < 1.0 ; x_0 += this->grid_step_size){
		x_1 = x_0 + this->grid_step_size;
		for (y_0 = -1.0; y_0 < 1.0; y_0 += this->grid_step_size){
			y_1 = y_0 + this->grid_step_size;

			this->do_square(x_0, x_1, y_0, y_1);
		

		}
		
	}

	//for (int i = 0; i < poly_data.tri_list.size(); i++)
		//std::cout << poly_data.tri_list[i] << " ";

	this->dirty = false;
	return true;
}

float interp(float x_s, float x_e, float v_s, float v_e){ //assumes interp zero
	return x_s + (-v_s / (v_e - v_s)) * (x_e - x_s);

}

void Marching::do_square(float x_0, float x_1, float y_0, float y_1){
	float corner_coords[8] = { x_0, y_0, x_1, y_0, x_1, y_1, x_0, y_1 };
	float corner_values[4];
	float intersect_coord[8] = { 0 };
	//int edge_pt_count = 0;

	for (int i = 0; i < 4; i++)
		corner_values[i] = this->evaluate(corner_coords[2 * i], corner_coords[2 * i + 1], 0);

	int square_index = 0;
	if (corner_values[0] > 0) square_index |= 1;
	if (corner_values[1] > 0) square_index |= 2;
	if (corner_values[2] > 0) square_index |= 4;
	if (corner_values[3] > 0) square_index |= 8;

	int* lines_edge = line_table[square_index];
	int alternative_idx = ambiguous_line_table_redirect[square_index];
	if (alternative_idx != -1){ //ambiguous case
		float midx = 0, midy = 0;
		for (int i = 0; i < 4; i++){
			midx += corner_coords[i * 2];
			midy += corner_coords[(i + 1) * 2];
		}
		midx /= 4.0; midy /= 4.0;
		float mid_val = this->evaluate(midx, midy, 0);
		if (mid_val < 0)
			lines_edge = line_table[alternative_idx];
	}
	for (int i = 0; i < 4; i ++){
		int line_pt = lines_edge[i];
		if (line_pt == -1) break;
		int vi1 = 2*line_pt;
		int vi2 = 2*((line_pt + 1) % 4);
		float x_interp = interp(corner_coords[vi1], corner_coords[vi2], corner_values[line_pt], corner_values[(line_pt + 1) % 4]);
		float y_interp = interp(corner_coords[vi1 + 1], corner_coords[vi2 + 1], corner_values[line_pt], corner_values[(line_pt + 1) % 4]);
		intersect_coord[2 * i] = x_interp;
		intersect_coord[2 * i + 1] = y_interp;
	}
	for (int i = 0; i < 2; i+=2){
		int line_pt = lines_edge[i];
		int line_pt2 = lines_edge[i+1];
		if (line_pt == -1) break;
		int pi1 = add_point(intersect_coord[2 * i], intersect_coord[2 * i + 1], 0);
		int pi2 = add_point(intersect_coord[2 * (i+1)], intersect_coord[2 * (i+1) + 1], 0);
		this->add_line(pi1, pi2);

	}




	/*
	for (int i1 = 0, i2 = 1; i1 < 4; i1++, i2 = (i1 + 1) % 4){
		float mult_result = corner_values[i1] * corner_values[i2];
		if (mult_result <= 0 && corner_values[i1] != corner_values[i2]) {
			//sign mismatch, and not both are zero
			float x_s = corner_coords[2 * i1], x_e = corner_coords[2 * i2];
			float y_s = corner_coords[2 * i1 + 1], y_e = corner_coords[2 * i2 + 1];
			float v_s = corner_values[i1], v_e = corner_values[i2];
			intersect_coord[2 * edge_pt_count] = x_s + (-v_s / (v_e - v_s)) * (x_e - x_s);//x intersect
			intersect_coord[2 * edge_pt_count + 1] = y_s + (-v_s / (v_e - v_s)) * (y_e - y_s);//y intersect
			edge_pt_count++;
		}

	}*/

	/*
	if (edge_pt_count == 2){
		this->add_line(intersect_coord[0], intersect_coord[1], intersect_coord[2], intersect_coord[3]);
	}
	else if (edge_pt_count == 4){
		float mid_pt1[2] = { (intersect_coord[0] + intersect_coord[2]) / 2, (intersect_coord[1] + intersect_coord[3]) / 2 };
		float mid_pt2[2] = { (intersect_coord[4] + intersect_coord[6]) / 2, (intersect_coord[5] + intersect_coord[7]) / 2 };
		float mid_of_mid[2] = { (mid_pt1[0] + mid_pt2[0]) / 2, (mid_pt1[1] + mid_pt2[1]) / 2 };
		float mid_val = this->evaluate(mid_of_mid[0], mid_of_mid[1], 0);
		float mult_result = mid_val * corner_values[0];
		if (mult_result > 0){ //sign matching first pt
			this->add_line(intersect_coord[0], intersect_coord[1], intersect_coord[2], intersect_coord[3]);
			this->add_line(intersect_coord[4], intersect_coord[5], intersect_coord[6], intersect_coord[7]);
		}
	}*/

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
	
	int new_vertex_i1 = add_point(x0, y0, -1);
	int new_vertex_i2 = add_point(x1, y1, -1);
	return add_line(new_vertex_i1, new_vertex_i2);

	

}
int Marching::add_line(int pi1, int pi2){

	int new_line_i = this->poly_data.tri_list.size();
	
	this->poly_data.tri_list.push_back(pi1);
	this->poly_data.tri_list.push_back(pi2);
	this->poly_data.tri_list.push_back(pi2);

	return new_line_i;

}


Poly_Data const * Marching::get_poly_data(){
	
	this->dirty = true;
	poly_data.tri_list.empty();
	poly_data.vertex_list.empty();

	poly_data.vertex_list.resize(18);
	poly_data.vertex_list[0] = .0;
	poly_data.vertex_list[1] = .0;
	poly_data.vertex_list[2] = .0;
	poly_data.vertex_list[3] = -1.0;
	poly_data.vertex_list[4] = -.0;
	poly_data.vertex_list[5] = .0;
	poly_data.vertex_list[6] = .0;
	poly_data.vertex_list[7] = 1.0;
	poly_data.vertex_list[8] = .0;
	poly_data.vertex_list[9] = .0;
	poly_data.vertex_list[10] = .0;
	poly_data.vertex_list[11] = .0;
	poly_data.vertex_list[12] = 1.0;
	poly_data.vertex_list[13] = .0;
	poly_data.vertex_list[14] = .0;
	poly_data.vertex_list[15] = .0;
	poly_data.vertex_list[16] = -1.0;
	poly_data.vertex_list[17] = .0;

	poly_data.tri_list.resize(6);	
	poly_data.tri_list[0] = 0;
	poly_data.tri_list[1] = 1;
	poly_data.tri_list[2] = 2;
	poly_data.tri_list[3] = 3;
	poly_data.tri_list[4] = 4;
	poly_data.tri_list[5] = 5;

	//if (this->dirty) this->recalculate();
	return &this->poly_data;
}
