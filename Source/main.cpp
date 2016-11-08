#include "marching.h"
#include "drawer.h"
#include "evaluator.h"


/**
 *	Main function
 */
int main(int argc, char **argv) 
{	
	Evaluator evaluator; //evaluates equations

	Marching march_maker; //generate the implicit surface mesh
	march_maker.set_evaluator(&evaluator);
	
	Drawer drawer(&argc, argv);	//draws the UI
	drawer.SetMarch(&march_maker);
	drawer.SetEvaluator(&evaluator);
	drawer.GetPolyData();
	drawer.start();		
}
