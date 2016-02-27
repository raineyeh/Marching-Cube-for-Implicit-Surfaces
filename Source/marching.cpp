#include "marching.h"
#include <iostream>


Marching::Marching(void){
	this->grid_step_size = .02;
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

	//....

	this->dirty = false;
	return true;
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
	
	return &this->poly_data;
}
