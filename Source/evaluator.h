#ifndef EVALUATOR_531_H
#define EVALUATOR_531_H
#include <iostream>
#include <string>
#include<cstring>
#include<cstdlib>
#include<vector>
#include<stack>
#include<algorithm>
#include <math.h>
#include "simple_stack.h"
/** An object that takes an input string (ie. "x^2+y-z^3) and can evaluate it for any x,y,z.
* The evaluate function will be called for every point of the marching cube vertices, and more.
*/

using namespace std;

class Evaluator {

private:
	vector<string>token;
	string equation;
	//stack<char>operator_stack;
	//stack<float>operand_stack;
	Simple_Stack<char> operator_stack;
	Simple_Stack<float> operand_stack;

	inline bool isoperator(char ch) { return ((ch == '+') || (ch == '-') || (ch == '*') || (ch == '/') || (ch == '^') || (ch == '(') || (ch == ')')); }
	inline bool is_open_brace(char ch) { return (ch == '('); }
	inline bool is_close_brace(char ch) { return (ch == ')'); }
	inline bool is_neg(char ch1, char ch2)
	{
		return (
			(ch1 == '-') && (isoperator(ch2) || is_open_brace(ch2)) ||
			((ch1 == '-') && (ch2==NULL) )
			);
	}
	bool is_number(char ch)
	{
		return  (ch >= '0' && ch <= '9') || ch == '.';
	}
	bool is_variable(char ch) { return  ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')); }
	int operator_precedence(char ch);
	void tokenizer();
	float evaluate_operation(char ch, float val1, float val2);
	void evaluate_op();
	bool check_bug();//parser bug

public:
	Evaluator();
	Evaluator(std::string);
	bool set_equation(std::string);
	float evaluate(float x, float y, float z);
	bool get_equation_from_file(std::string &str);
	bool save_equation_to_file();
	
};

#endif