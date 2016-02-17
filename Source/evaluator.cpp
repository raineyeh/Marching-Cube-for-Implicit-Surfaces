#include "evaluator.h"

Evaluator::Evaluator(){

}

Evaluator::Evaluator(std::string s){
	set_equation(s);
}

bool Evaluator::set_equation(std::string s){
	return false;
}

float Evaluator::evaluate(float x, float y, float z){
	return x*x + y*y; //dub. temporary
}