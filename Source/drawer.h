#include "marching.h"

class Drawer{
public:
	Drawer(int*, char**);
	bool set_march(Marching*);
	bool Get_poly_data();
	void start();
	void SendData(float);
	void NextStep();
private:
	Marching* marching_obj;

};