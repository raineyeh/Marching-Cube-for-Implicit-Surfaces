#include "marching.h"

class Drawer{
public:
	Drawer(int*, char**);
	bool set_march(Marching*);
	void start();

private:
	Marching* marching_obj;

};