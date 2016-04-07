/**
 *	
 */


#include "marching.h"
#include "drawer.h"
#include "evaluator.h"
#include <iostream>

//#include "marching_test_drawer.h"


/**
 *	Main function
 */
int main(int argc, char **argv) 
{	
	Evaluator evaluator;
	Marching march_maker;
	march_maker.set_evaluator(&evaluator);
	evaluator.set_equation("x+0.01");
	
	//march_maker.set_grid_step_size(0.05);
	//march_maker.set_seed(0, 0.0, 0.0);
	//march_maker.seed_mode(true);
	//test_marching(&march_maker);

	Drawer drawer(&argc, argv);	
	drawer.SetMarch(&march_maker);
	drawer.SetEvaluator(&evaluator);
	drawer.start();		
}
