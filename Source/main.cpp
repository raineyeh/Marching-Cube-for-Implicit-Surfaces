/**
 *	
 */


#include "marching.h"
#include "drawer.h"
#include "evaluator.h"

#include <iostream>



/**
 *	Main function
 */
int main(int argc, char **argv) 
{
	
	Evaluator evaluator;
	evaluator.set_equation("x^2+y"); //this should be called by the drawer as user input

	Marching march_maker;
	march_maker.set_evaluator(&evaluator);

	Drawer drawer(&argc, argv);
	drawer.set_march(&march_maker);
	drawer.start();
	
}
