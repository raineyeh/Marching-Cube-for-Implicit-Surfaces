
#include "marching.h"
#include "drawer.h"
#include "evaluator.h"


/**
 *	Main function
 */
int main(int argc, char **argv) 
{	
	Evaluator evaluator;
	Marching march_maker;
	march_maker.set_evaluator(&evaluator);
	
	Drawer drawer(&argc, argv);	
	drawer.SetMarch(&march_maker);
	drawer.SetEvaluator(&evaluator);
	drawer.GetPolyData();
	drawer.start();		
}
