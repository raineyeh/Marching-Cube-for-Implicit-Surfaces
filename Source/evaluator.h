/** A class that takes an input string (eg. "x^2+y-z^3) and can evaluate it for any x,y,z.
* The evaluate function will be called for every point of the marching cube vertices, and more...
*/

#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <stack>
#include <algorithm>
#include <math.h>
#include "simple_stack.h"

using namespace std;

//type of tokens
// in order listed: operators, numbers, variables, open parenthesis '(', 
//   close parenthesis ')', negative sign '-' (different from minus sign), and none of the above
enum TOK_TYPE { OP, NUM, VAR, BRAC_O, BRAC_C, NEG, NONE }; 


class Evaluator {
private:
	vector<string>token; //list of tokens parsed from the input equation
	vector<TOK_TYPE> token_type; //token type for each token in tokens
	bool tokenized; //true if the equation is parsed into tokens
	string equation; //string of input equation. Should always contain a valid equation string.
	
	//stacks used during evaluation. Use Simple_Stack for speed.
	//stack<char>operator_stack;
	//stack<float>operand_stack;
	Simple_Stack<char> operator_stack;
	Simple_Stack<float> operand_stack;

	inline bool is_operator(char ch) { return ((ch == '+') || (ch == '-') || (ch == '*') || (ch == '/') || (ch == '^') ); }
	inline bool is_open_brace(char ch) { return (ch == '('); }
	inline bool is_close_brace(char ch) { return (ch == ')'); }
	bool is_number(char ch){ return  (ch >= '0' && ch <= '9') || ch == '.'; }
	bool is_variable(char ch) { return  ((ch >= 'x') && (ch <= 'z')) || ((ch >= 'X') && (ch <= 'Z')); }
	int operator_precedence(char ch); //returns a number indicating operator precedence
	
	bool tokenize(string eq_str); //break the equation string into token. 
								  //Return true if successful, false if there's an error. 
								  //Equation string is unchanged if there's an error.
	bool tokenize() { return tokenize(this->equation); } //tokenize using the last known working equation

	float evaluate_operation(char ch, float val1, float val2); // given an operator and 2 values, evaluate them
	void evaluate_op(); // evaluate using the operator on the top of the operator_stack

	//for debugging
	void print_tokens() { for (auto i = 0; i < this->token.size(); i++) cout << i << ":" << token[i] << endl; };

public:
	Evaluator(); //default constructor with a dummy equation
	Evaluator(string); //constructor given an equation. Will throw exception if equation fail to parse.

	bool set_equation(std::string); //attempt to use the input equation. If there's parse error, the equation is not set.

	float evaluate(float x, float y, float z); //evaluate the given x,y,z value using the last set working equation

	bool get_equation_from_file(std::string &str); // read an equation string from file
	bool save_equation_to_file(); //save the current equation string to file

	// test tokenizing with some test cases.
	void test() { 
		check_parse("-(x+ -(y)* -.021)", 1);
		check_parse("(x(y)", 0);
		check_parse("(x)", 1);
		check_parse("(x-)", 0);
		check_parse("(-x)", 1);
		check_parse("-(-x)", 1);
		check_parse("", 0);
		check_parse("xyz", 1);
		check_parse("xy/z^-.22", 1);
	}

	void check_parse(string str, bool expect_success) {
		bool tok_ret = tokenize(str);
		if (tok_ret == expect_success)
			cout << "PASS eq:" << str << endl;
		else
			cout << "FAIL eq:" << str << "\t expect " << expect_success <<", got "<< tok_ret <<endl;
	}
};

