#ifndef MARCHING_LOOKUP_531_H
#define MARCHING_LOOKUP_531_H

int line_table[][4] = {
	{ -1, -1, -1, -1 }, //0
	{ 0, 3, -1, -1 }, //1
	{ 0, 1, -1, -1 }, //2
	{ 1, 3, -1, -1 }, //3
	{ 1, 2, -1, -1 }, //4
	{ 0, 1, 2, 3 }, //5 ambiguous case
	{ 0, 2, -1, -1 }, //6
	{ 2, 3, -1, -1 }, //7
	{ 3, 4, -1, -1 }, //8
	{ 0, 2, -1, -1 }, //9
	{ 0, 3, 1, 2 }, //10 ambiguous case
	{ 1, 2, -1, -1 }, //11
	{ 1, 3, -1, -1 }, //12
	{ 0, 1, -1, -1 }, //13
	{ 0, 3, -1, -1 }, //14
	{ -1, -1, -1, -1 }, //15
};

int ambiguous_line_table_redirect[] = { // if midpoint is neg (does not match v0), then use the new lookup value
	-1, -1, -1, -1, -1, 5, -1, -1, -1, -1, 10, -1, -1, -1, -1, -1
};



#endif