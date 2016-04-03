#pragma once

#include <iostream>
#include <vector>

template<class T>
class Simple_Stack
{
public:
	Simple_Stack(){ max_size = 50; item_arr.resize(max_size); num_items = 0; }

	void push(T item){ 
		//cout << "push " << item << endl;
		if (max_size == num_items){
			max_size += 50;
			item_arr.resize(max_size);
		}
		item_arr[num_items] = item; 
		num_items++;
	}
	void pop(){ 
		//cout << "pop " << item_arr[num_items-1] << endl;
		num_items--; 

		/*if (num_items < 0)
			cout << "something is wrong" << endl;*/
	}
	bool empty(){ return this->num_items == 0; }
	T top(){ return item_arr[num_items-1]; }
	void clear(){ num_items = 0; }
private:
	std::vector<T> item_arr;
	int num_items;
	int max_size;
};

