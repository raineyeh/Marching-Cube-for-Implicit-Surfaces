#include "evaluator.h"
#include <iostream>
#include <windows.h>
#include <fstream>
#include <iostream>
using namespace std;

Evaluator::Evaluator()
{
	equation = "x+y";
	token.clear();
}

Evaluator::Evaluator(std::string s) {
	set_equation(s);
}

bool Evaluator::set_equation(std::string s) {
	string old_eq = equation;
	equation = s;
	tokenizer();

	if (!check_bug()){
		equation = old_eq;
		tokenizer();
		return false;
	}

	//not sure this is needed. both stacks should be empty already. -R
	while (!operator_stack.empty())
	{
		operator_stack.pop();
	}
	while (!operand_stack.empty())
	{
		operand_stack.pop();
	}
	return true;
}
bool Evaluator::check_bug()
{
	if ( token.size() == 0) { 
	//	std::cout<< "empty string" << endl; 
		return false; 
	}
	int open_brace = 0;
	for (int i = 0; i < token.size(); i++)
	{ 
		char ch = token[i].at(0);
		if (is_open_brace(ch)){
			open_brace++;
		}
		else if (is_close_brace(ch)){
			open_brace--;
			if (open_brace == -1){
				//std::cout << "wrong braces" << endl; 
				return false;
			}
		}
		
	}//end for

	if (open_brace == 0) return true;
	else {
		std::cout << "wrong braces" << endl; return false;
	}
}

void Evaluator::evaluate_op(){
	char temp = operator_stack.top(); operator_stack.pop();  //cout << "pop opt stack" << temp << endl;
	float val1 = operand_stack.top(); operand_stack.pop();   // cout << "pop op stack" << val1 << endl;
	float val2 = operand_stack.top(); operand_stack.pop();   // cout << "pop opt stack" << val2 << endl;
	float result = evaluate_operation(temp, val2, val1);     //  cout << "pop opt stack" << val2 << endl;
	operand_stack.push(result);
}


float Evaluator::evaluate(float x, float y, float z) {
	//start evaluate 

//		if (!(check_bug())) return NAN; //moved to set_equation()

		for (int i = 0; i < token.size(); i++)
		{//start of for
			int tok_size = token[i].size();
			char ch1 = token[i].at(0);
			char ch2;
			if (tok_size>1)
				ch2 = token[i].at(1);

			//cout << ch1<< is_number(ch1)<<endl;
			if (is_variable(ch1))
			{
				if (ch1 == 'x' || ch1 == 'X') {
					operand_stack.push(x);  //std::cout << "stack so from var  " << operand_stack.top() << endl;
				}
				else if (ch1 == 'y' || ch1 == 'Y') {
					operand_stack.push(y); //std::cout << "stack so from var " << operand_stack.top() << endl;
				}
				else if (ch1 == 'z' || ch1 == 'Z') {
					operand_stack.push(z); //std::cout << "stack so from var  " << operand_stack.top() << endl;
				}
				else
				{
					std::cout <<"'"<< ch1 << "' is not an accepted variable ";
					return NAN;
				}
			}
			else if ((tok_size == 2) && (ch1 == '-') && is_variable(ch2))	{
				if (ch2 == 'x' || ch2 == 'X') {
					operand_stack.push(-1 * x);  //std::cout << "stack so from var  " << operand_stack.top() << endl;
				}
				else if (ch2 == 'y' || ch2 == 'Y') {
					operand_stack.push(-1 * y); //std::cout << "stack so from var " << operand_stack.top() << endl;
				}
				else if (ch2 == 'z' || ch2 == 'Z') {
					operand_stack.push(-1 * z); //std::cout << "stack so from var  " << operand_stack.top() << endl;
				}
				else
				{
					std::cout << "'" << ch1 << "' is not an accepted variable ";
					return NAN;
				}
			}

			else if ((tok_size >= 1) && (is_number(ch1) || is_number(ch1)))
			{
				std::string::size_type sz = tok_size;
				float  d = stof(token[i], &sz);
				//cout << "d="<< d << endl;
				operand_stack.push(d);
				//std::cout << "stack ss from number stof   = " << operand_stack.top() << endl;
			}

			else if (isoperator(ch1) && tok_size == 1)
			{//start evaluating when operators arrive 
				//cout << ch1 <<" "<< isoperator(ch1) <<" "<< endl;
				//case open brace 
				if (is_open_brace(ch1))
					operator_stack.push(ch1);
				//this part for testing closed brace we will do every calcualtion until the open brace
				else if (is_close_brace(ch1))
				{
					//cout << "hi close brace" << endl;


					while (!(is_open_brace(operator_stack.top())))
					{//start while
						evaluate_op();
					}//end while
					operator_stack.pop();
				}//end of closed brace 

				else if (isoperator(ch1) && !(is_open_brace(ch1)) && !(is_close_brace(ch1)))
				{//start other operator if
					while (true)
					{//start of while
						if ((operator_stack.empty()) || (operator_precedence(ch1) > operator_precedence(operator_stack.top())
							|| (is_open_brace(operator_stack.top()))))
						{
							operator_stack.push(ch1); //cout << "operator_stack_push case " << ch1 << endl;
							break;
						}
						//if (operator_stack.empty() || operand_stack.size() < 2) { cout << "no operand  operator are available" << endl; break;  return NAN; }
						evaluate_op();
					}//end of while
				}// 
			}//
		} //
		
		while (!operator_stack.empty()) {
			evaluate_op();
		}//end of while

		//cout << "the final result of the equation =" << operand_stack.top();
		float result = operand_stack.top();
		operand_stack.pop();

		return result;
	
} //end evaluater 



int Evaluator::operator_precedence(char ch){
	switch (ch)
	{
	case '^': return 3;
	case '/': return 2;
	case '*': return 2;
	case '+': return 1;
	case '-': return 1;
	case '(': return 0;
	case ')': return 0;
	default: return -1;  //cout << "no such operation " << endl;
	}
}

float Evaluator::evaluate_operation(char ch, float val1, float val2){
	 switch (ch){
	 case '+': return val1 + val2;
	 case '-': return val1 - val2;
	 case '*': return val1 * val2;
	 case '/': return val1 / val2;
	 case '^': return pow(val1, val2);
	 }
}

void Evaluator::tokenizer()
{
	token.clear();
	//to remove any spaces from the equation 
	equation.erase(std::remove(equation.begin(), equation.end(), ' '), equation.end());
	//now no spaces in equation 
	//the equation for the checking of marching cubes 
	// the result of the tokenization based on finding the operation 
	int curr1 = 0;       //index to keep track of last split 
	//cout << equation << endl;
	int i;
	if (equation[0] == '-')  i = 1; else i = 0;
	for (; i<equation.size(); i++)        //we trace the equation character by character 
	{//start for
	 //cout << i <<" "<<equation[i]<< "   this is boolean " << isoperator(equation[i]) << endl;
	 if ( i==0 && isoperator(equation[0]) && (equation[0] != '-'))
	 {
		 token.push_back(equation.substr(0, 1));
		 curr1 = 1;
	 }
	 if (i>0 && isoperator(equation[i]) && (!is_neg(equation[i], equation[i - 1])))      //if the char is an operator but not a negative sign (this is to differntiate minus from negative )
		{
			//cout << i << " " << isoperator(equation[i]) << endl;

			if (equation.substr(curr1, i - curr1) != "")
			{
				token.push_back(equation.substr(curr1, i - curr1)); 
			}
			if (equation.substr(i, 1) != "")
			{
				token.push_back(equation.substr(i, 1)); 
			}
						curr1 = i + 1;
		}//end if 
	 }//end for
	if (i > curr1)
		token.push_back(equation.substr(curr1));                   //insertion to the	
																  /* cout << "size of the vector token " << token.size() << endl;
																   for (int i = 0; i < token.size(); i++)
																   	cout << token[i] << endl;*/
}//end of tokenizer


bool Evaluator::get_equation_from_file(std::string &str)
{

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



