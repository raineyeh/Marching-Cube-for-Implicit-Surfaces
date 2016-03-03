#include "evaluator.h"

Evaluator::Evaluator() {
	set_equation("x^2+y^2-0.5");
}

Evaluator::Evaluator(std::string s) {
	set_equation(s);
}

bool Evaluator::set_equation(std::string s) {
	equation = s;
	tokenizer();
	return true;
}

float Evaluator::evaluate(float x, float y, float z) {
	{//start evaluate 

		for (int i = 0; i < token.size(); i++)
		{//start of for
			char ch = token[i].at(0);
			//cout << ch<< is_number(ch)<<endl;
			if (is_variable(ch))
				if (ch == 'x' || ch == 'X') {
					operand_stack.push(x);  //cout << "stack so from var  "<<operand_stack.top() << endl;
				}
				else if (ch == 'y' || ch == 'Y') {
					operand_stack.push(y);// cout << "stack so from var "   <<operand_stack.top() << endl;
				}
				else if (ch == 'z' || ch == 'Z') {
					operand_stack.push(z);// cout << "stack so from var  "  << operand_stack.top() << endl;
				}
				else
					cout << "not an accepted variable ";
				if (is_number(ch) || token[i].size() >= 2)
				{
					std::string::size_type sz = token[i].size();
					float  d = stof(token[i], &sz);
					//cout << "d="<< d << endl;
					operand_stack.push(d);
					//cout << "stack ss from number  = " << operand_stack.top() << endl;
				}
				if (isoperator(ch) && token[i].size() == 1)
				{//start evaluating when operators arrive 
				 //cout << ch <<" "<< isoperator(ch) <<" "<< endl;
				 //case open brace 
					if (is_open_brace(ch))
						operator_stack.push(ch);
					//this part for testing closed brace we will do every calcualtion until the open brace
					if (is_close_brace(ch))
					{
						//cout << "hi close brace" << endl;


						while (!(is_open_brace(operator_stack.top())))
						{//start while
							char temp = operator_stack.top(); operator_stack.pop();  //cout << "pop opt stack" << temp << endl;
							float val1 = operand_stack.top(); operand_stack.pop();   // cout << "pop op stack" << val1 << endl;
							float val2 = operand_stack.top(); operand_stack.pop();   // cout << "pop opt stack" << val2 << endl;
							float result = evaluate_operation(temp, val2, val1);     //  cout << "pop opt stack" << val2 << endl;
							operand_stack.push(result);
						}//end while
						operator_stack.pop();
					}//end of closed brace 

					if (isoperator(ch) && !(is_open_brace(ch)) && !(is_close_brace(ch)))
					{//start other operator if
						while (true)
						{//start of while
							if ((operator_stack.empty()) || (operator_precedence(ch) > operator_precedence(operator_stack.top())
								|| (is_open_brace(operator_stack.top()))))
							{
								operator_stack.push(ch); //cout << "operator_stack_push case " << ch << endl;
								break;
							}

							char temp = operator_stack.top(); operator_stack.pop();   //cout << "pop op stack" << temp << endl;
							float val1 = operand_stack.top(); operand_stack.pop(); //cout << "pop oprand stack" << val1 << endl;
							float val2 = operand_stack.top(); operand_stack.pop();  //cout << "pop opt stack" << val2 << endl;
							float result = evaluate_operation(temp, val2, val1);
							operand_stack.push(result);   //cout << "push result stack" << result << endl;
						}//end of while
					}// 
				}//
		} //
		char temp; float val1, val2, result;
		while (!operator_stack.empty()) {
			temp = operator_stack.top();   	operator_stack.pop();   //cout << "pop op stack" << temp << endl;
			val1 = operand_stack.top();		operand_stack.pop();   //cout << "pop oprand stack" << val1 << endl;
			val2 = operand_stack.top();		operand_stack.pop();   // cout << "pop opt stack" << val2 << endl;
			result = evaluate_operation(temp, val2, val1);
			operand_stack.push(result);
		}//end of while

		//cout << "the final result of the equation =" << operand_stack.top();
		return operand_stack.top();
	} //end evaluater 
}
int Evaluator::operator_precedence(char ch)
{
	switch (ch)
	{
	case '^': return 3; break;
	case '/': return 2; break;
	case '+':return 1; break;
	case '-': return 1; break;
	case '(':return 0; break;
	case ')':return 0; break;
	default:return -1 ;  //cout << "no such operation " << endl;
	}
}
 float Evaluator::evaluate_operation(char ch, float val1, float val2)
{
	if (ch == '+') return val1 + val2;
	if (ch == '-') return val1 - val2;
	if (ch == '*') return val1 * val2;
	if (ch == '/') return val1 / val2;
	if (ch == '^') return pow(val1, val2);
}
void Evaluator::tokenizer()
{
	//to remove any spaces from the equation 
	equation.erase(std::remove(equation.begin(), equation.end(), ' '), equation.end());
	//now no spaces in equation 
	//the equation for the checking of marching cubes 
	// the result of the tokenization based on finding the operation 
	int curr1 = 0;       //index to keep track of last split 
	//cout << equation << endl;
	int i = 1;
	for (i = 1; i<equation.size(); i++)        //we trace the equation character by character 
	{//start for
	 //cout << i <<" "<<equation[i]<< "   this is boolean " << isoperator(equation[i]) << endl;
		if (isoperator(equation[i]) && (!is_neg(equation[i], equation[i - 1])))      //if the char is an operator but not a negative sign (this is to differntiate minus from negative )
		{
			//cout << i << " " << isoperator(equation[i]) << endl;

			if (equation.substr(curr1, i - curr1) != "")
				token.push_back(equation.substr(curr1, i - curr1));
			if (equation.substr(i, 1) != "")
				token.push_back(equation.substr(i, 1));
			curr1 = i + 1;
		}//end if 
	}//end for
	if (i > curr1)
		token.push_back(equation.substr(curr1));                   //insertion to the															   //cout << "size of the vector" << token.size() << endl;
																   //for (int i = 0; i < token.size(); i++)
																   //	cout << token[i] << endl;
}//end of tokenizer

