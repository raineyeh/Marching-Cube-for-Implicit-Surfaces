#ifndef EVALUATOR_531_H
#define EVALUATOR_531_H
#include <string>

/** An object that takes an input string (ie. "x^2+y-z^3) and can evaluate it for any x,y,z.
* The evaluate function will be called for every point of the marching cube vertices, and more.
*/

class Evaluator{

public:
	Evaluator();
	Evaluator(std::string);
	bool set_equation(std::string);
	float evaluate(float x, float y, float z);



};

#endif