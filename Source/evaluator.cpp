#include "evaluator.h"
#include <windows.h>
#include <fstream>
using namespace std;

Evaluator::Evaluator(){
	set_equation("x+y"); //put a default equation
}

Evaluator::Evaluator(std::string s){
	if(!set_equation(s))
		throw exception(); // throw exception if there's parse error for the input equation
}

bool Evaluator::set_equation(std::string s) {
	return tokenize(s);
}

/* Look at the top of the operand stack and attempts to evaluate it. 
If the next in the stack has higher operating precedence, it will be evaluated first
by recursive all to this function. */
void Evaluator::evaluate_op(){
	char op = operator_stack.top(); operator_stack.pop();
	
	// an operator that takes 2 values (+,-,*,/,^)
	if (is_operator(op)) {
		float val1 = operand_stack.top(); operand_stack.pop(); //get the first value

		//check to see if the next operator has a higher operator precedence.
		//if so, evaluate it first. 
		//eg. * and / has a higher operating precedence than + and -, and will be evaluated first.
		if (!operator_stack.empty()) {
			char op2 = operator_stack.top();
			if (operator_precedence(op2) > operator_precedence(op)) {
				evaluate_op();
			}
		}
		float val2 = operand_stack.top(); operand_stack.pop(); //get the 2nd value
		float result = evaluate_operation(op, val2, val1); //evaluate val_1 [op] val_2
		operand_stack.push(result); //save the result on the operand stack
	}
	else if (op == 'N') { //negative sign. negate the value on the top of operand_stack
		float val = operand_stack.top(); operand_stack.pop();
		val = -val;
		operand_stack.push(val);
	}
	else throw exception(); //unknown operation. Shouldn't reach here
}

/* Given x,y,z, evaluate their value on the equation by traversing the tokens
and popping them on an operator/operand stack, and evaluating the stack when possible.
Assume no parse error. */
float Evaluator::evaluate(float x, float y, float z) {
	if (!tokenized) tokenize(); //make sure the equation string is tokenized

	TOK_TYPE tok_type; //type of token
	string tok; //token
	char ch; //first character in the token, as most tokens will only use the first character
	float num;

	//traverse each token
	for (auto i = 0; i < token.size(); i++) {
		tok_type = this->token_type[i];
		tok = this->token[i];
		ch = tok[0];
		
		switch (tok_type){
		case NEG: //negative '-' (not to be confused with minus operation)
			operator_stack.push('N');
			break;
		case VAR: //a variable (x,y,z)
			if (ch == 'x' || ch == 'X')
				operand_stack.push(x);
			else if (ch == 'y' || ch == 'Y') 
				operand_stack.push(y);
			else if (ch == 'z' || ch == 'Z') 
				operand_stack.push(z);
			else
				throw exception(); //shouldn't reach here because the tokenize() would have errored.
			break;
		case NUM: //a number
			num = stof(token[i]);
			operand_stack.push(num);
			break;
		case BRAC_O: // open bracket '('
			operator_stack.push(ch);
			break;
		case BRAC_C: // close bracket ')'
			//evaluate everything within the bracket
			while (!this->is_open_brace(operator_stack.top()))
				evaluate_op();
			operator_stack.pop(); //pop the '('
			break;
		case OP: // operator +,-,*,/,^
			operator_stack.push(ch);
			break; 
		} //end switch
	} 
	//finished traversing the tokens. evaluate everything on the stack
	while (!operator_stack.empty()) {
		evaluate_op();
	}

	//final result of the evaluation
	float result = operand_stack.top();  operand_stack.pop();
	return result;
} //end evaluater 


// return the order to which operation should be carried out
int Evaluator::operator_precedence(char ch){
	switch (ch)
	{
	case 'N': return 4;
	case '^': return 3;
	case '/': return 2;
	case '*': return 2;
	case '+': return 1;
	case '-': return 1;
	case '(': return 0;
	case ')': return 0;
	default: throw exception(); //unknown operation. shouldn't reach here.
	}
}

// given an operation and 2 values, calculate.
float Evaluator::evaluate_operation(char ch, float val1, float val2){
	 switch (ch){
	 case '+': return val1 + val2;
	 case '-': return val1 - val2;
	 case '*': return val1 * val2;
	 case '/': return val1 / val2;
	 case '^': return pow(val1, val2);
	 default: throw exception(); //unknown operation. shouldn't reach here.
	 }
}

/* Break up the equation string into tokens */
bool Evaluator::tokenize( string eq_str) {
	this->tokenized = false;
	if (eq_str.empty()) return false;

	token.clear();
	token_type.clear();

	//remove any spaces from the input equation 
	eq_str.erase(remove(eq_str.begin(), eq_str.end(), ' '), eq_str.end());

	bool neg = false; //switch if parsing a negative sign (not minus)
	int brac_count = 0;
	TOK_TYPE last_tok = NONE;
	

//	int curr1 = 0;       //index to keep track of last split 
	for (auto i = 0; i < eq_str.size(); i++) {
		char ch = eq_str[i];
		string s(1, ch);
		//cout << i <<" "<<eq_str[i]<< "   is_operator:" << is_operator(eq_str[i]) << endl;
		
		// Check for negative. A negative occurs when '-' is the first character in the equation, 
		// or if the preceeding character is an operator or a '('
		if (ch == '-' && (i == 0 || eq_str[i - 1] == '(' || is_operator(eq_str[i - 1]))) {
			if (neg) return false; //double negative 
			neg = true; 
			token.push_back("NEG");
			token_type.push_back(NEG);
			continue;
		}

		else if (is_open_brace(ch)) {
			// if the previous token is variable, number, or ")", assume multiply
			if (last_tok == VAR || last_tok == NUM || last_tok == BRAC_C) {
				token.push_back("*");
				token_type.push_back(OP);
			}
			token.push_back(s);
			brac_count++;
			last_tok = BRAC_O;
		}
		else if (is_close_brace(ch)) {
			if (neg || last_tok == BRAC_O || last_tok == OP) return false; // "-)", "()", "[op])" aren't valid
			if (brac_count == 0) return false; // close bracket encountered before open bracket
			token.push_back(s);
			brac_count--;
			last_tok = BRAC_C;
		}
		else if (is_operator(ch)) {
			// negative or operator followed by an operator is invalid
			if (neg || last_tok == BRAC_O || last_tok == OP || last_tok == NONE) return false;
			token.push_back(s);
			last_tok = OP;
		}
		else if (is_number(ch)) {
			if (last_tok == VAR || last_tok == BRAC_C) { //previous token is a variable or ')'. assume multiply
				token.push_back("*");
				token_type.push_back(OP);
			}
			bool dot = ch == '.'; //see a dot 
			while (i+1 < eq_str.size()) {
				if (is_number(eq_str[i + 1])) {
					if (eq_str[i + 1] == '.') {
						if (dot) return false; //see 2 dots in this number, which is invalid number
						dot = true;
					}
					s += eq_str[++i];
				}
				else break;
			}
			
			if (s == ".") return false; //make sure the number isn't just a '.'
			
			token.push_back(s);
			last_tok = NUM;
		}
		else if (is_variable(ch)) {
			// if before the variable it's another variable, a number, or a ")", assume multiply
			if (last_tok == VAR || last_tok == NUM || last_tok == BRAC_C) {
				token.push_back("*");
				token_type.push_back(OP);
			}
			token.push_back(s);
			last_tok = VAR;
		}
		else return false; //encountered unknown character

		token_type.push_back(last_tok);
		neg = false;
	} //end of traversing the equation string

	//check the number of bracket is correct
	if (brac_count != 0) return false;

	//tokenize successfully
	this->equation = eq_str;
	this->tokenized = true;
	return true;
}

// Open the window file read menu, and attempts to read an equation from file.
bool Evaluator::get_equation_from_file(std::string &str) {
	FILE *fp;
	OPENFILENAME OpenFilename;
	TCHAR	szFile[MAX_PATH] = "equation.txt";
	TCHAR	szTitle[MAX_PATH] = "points";
	static TCHAR szFilter[] = "file type\0*.*\0file type1\0*.*\0";
	OpenFilename.lStructSize = sizeof(OPENFILENAME);
	OpenFilename.hwndOwner = NULL;
	OpenFilename.hInstance = NULL;
	OpenFilename.lpstrFilter = szFilter;
	OpenFilename.lpstrCustomFilter = NULL;
	OpenFilename.nMaxCustFilter = 0;
	OpenFilename.nFilterIndex = 0;
	OpenFilename.lpstrFile = szFile;
	OpenFilename.nMaxFile = MAX_PATH;
	OpenFilename.lpstrFileTitle = szTitle;
	OpenFilename.nMaxFileTitle = MAX_PATH;
	OpenFilename.lpstrInitialDir = "."; //Initial directory
	OpenFilename.lpstrTitle = "Load Equation";
	OpenFilename.Flags = OFN_EXPLORER;
	OpenFilename.nFileOffset = 0;
	OpenFilename.nFileExtension = 0;
	OpenFilename.lpstrDefExt = TEXT("txt");
	OpenFilename.lCustData = 0L;
	OpenFilename.lpfnHook = NULL;
	OpenFilename.lpTemplateName = NULL;
	str = this->equation;

	if (!GetOpenFileName(&OpenFilename))
		return false;
	errno_t err = fopen_s(&fp, OpenFilename.lpstrFile, "r");
	if (NULL == fp)
		return  false;
	char char_arr[256];
	fgets(char_arr, _countof(char_arr), fp);

	bool b = set_equation(str);
	if (b){
		str = string(char_arr);
	}

	fclose(fp);
	return b;
}


bool Evaluator::save_equation_to_file()
{
	FILE *fp;
	OPENFILENAME OpenFilename;
	TCHAR	szFile[MAX_PATH] = "equation.txt";
	TCHAR	szTitle[MAX_PATH] = "points";
	static TCHAR szFilter[] = "file type\0*.*\0file type1\0*.*\0";
	OpenFilename.lStructSize = sizeof (OPENFILENAME);
	OpenFilename.hwndOwner = NULL;
	OpenFilename.hInstance = NULL;
	OpenFilename.lpstrFilter = szFilter;
	OpenFilename.lpstrCustomFilter = NULL;
	OpenFilename.nMaxCustFilter = 0;
	OpenFilename.nFilterIndex = 0;
	OpenFilename.lpstrFile = szFile;
	OpenFilename.nMaxFile = MAX_PATH;
	OpenFilename.lpstrFileTitle = szTitle;
	OpenFilename.nMaxFileTitle = MAX_PATH;
	OpenFilename.lpstrInitialDir = ".";
	OpenFilename.lpstrTitle = "Save Equation";
	OpenFilename.Flags = OFN_EXPLORER;
	OpenFilename.nFileOffset = 0;
	OpenFilename.nFileExtension = 0;
	OpenFilename.lpstrDefExt = TEXT("txt");
	OpenFilename.lCustData = 0L;
	OpenFilename.lpfnHook = NULL;
	OpenFilename.lpTemplateName = NULL;

	if (GetSaveFileName(&OpenFilename) == false)
		return false;

	errno_t err = fopen_s(&fp, OpenFilename.lpstrFile, "w");

	if (NULL == fp)
		return false;
	if (equation == ""){
		fclose(fp);
		return false;
	}

	int ret = fprintf_s(fp, "%s", equation.c_str());
	//cout << equation << endl;

	fclose(fp);
	return ret != EOF;
		
}



