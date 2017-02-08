/*
A self-contained calculator object.
	Input: strings representing mathematical operations or non-real-valued functions.
	Output: numbers or non-real-valued operations.
*/

#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

const unsigned short int NUM_CONSTANTS = 5;
const string CONSTANT_NAMES[NUM_CONSTANTS] = { "pi", "phi", "e", "g", "G" };
const long double CONSTANT_VALUES[NUM_CONSTANTS] =
{
	acos(-1.0L),
	0.5L * (1.0L + sqrt(5.0L)),
	2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274274663919320030599218174135966290435729003342952605956307381323286279434907632338298807531952510190115738341879307021540891499348841675092447614606680822648001684774118537423454424371075390777449920695517027618386062613313845830007520449338265602976067371132007093287091274437470472306969772093101416928368190255151086574637721112523897844250569536967707L,
	9.80665,
	6.67408
};

class Calculator
{
public:

	Calculator();
	~Calculator();
	bool calculate(string input); //Returns whether to seek answer or not.
	bool get_quit() const;
	long double get_answer() const;
	void print_operations();
	void output(string out);
	void output(long double out);

protected:

	///SHOULD I MAKE OBJECTS INSTEAD OF JUST A LOG???
	fstream log_file;
	bool keep_log;
	long double answer;
	bool bad_input;
	bool quit;
	bool is_in_abs;
	bool is_radians;
	vector<string> previous_input;		 //Includes bad inputs.
	vector<long double> previous_answer; //No bad answers; not same size as previous_input!
	vector<string> variable_names;
	vector<long double> variable_values;

	//NOTE: each function that can't use something it reads puts it back, unless invalid.

	//TOOLS
	void log_add(string out);
	void load_settings();
	void save_settings();
	void pre_check(stringstream& ss);				 //add ans, remove spaces, \n's, @.
	void variable_assign(const string& name, const long double& value);

	bool check_in_between(stringstream& ss);		 //Checks for &(@) and executes it.
	bool check_end(stringstream& ss);				 //Also calls for &(@) check, recursively.
	string get_function_name(stringstream& ss);		 //Needs Upper, lower..., "(".
	string get_variable_name(stringstream& ss);      //All lower case. Also use for constants.
	vector<string> get_arguments(stringstream& ss);	 //Collects the inside of a function's ()'s.
	string get_parenthetical(stringstream& ss);		 //Same, but requires '(' type.

	bool is_integer(const long double& value) const; //Returns whether value is integer.
	long int int_factorial(long int value) const;
	long double compute_factorial(const long double& value);

	//SUB-NUMBER LEVEL
	long double get_number(stringstream& ss);        //Actual number reading, with decimal.
	long double read_function(stringstream& ss);     //Read real-valued function.
	const long double* read_variable(stringstream& ss);//Includes a search for constants.

	//REAL-VALUED NUMBER LEVEL
	long double read_number(stringstream& ss);       //#, (), var, real-val func, NOT ! or E.

	//NUMBER OBJECT LEVEL
	long double read_number_object(stringstream& ss);//Includes ! and E.
	//long int read_integer_object(stringstream& ss);  //Includes ! and E.
		////ADD ^ SOMEWHERE!!!!
	//TERM LEVEL
	long double read_term(stringstream& ss);         //Reads, executes, and returns a term.

	//MULTI-TERM LEVEL
	bool read_nonreal(stringstream& ss, bool allow_after);
		//Returns if next piece is a non-real-valued function and executes it,
		//also will return bad input if there is anything after unless inside an in-betweener.
	bool read_assignment_or_boolean(stringstream& ss, bool allow_after);
		//Returns if next piece is an assignment or a boolean and executes it,
		//also will return bad input if there is anything after unless inside an in-betweener,
		//and won't change answer if in an in-betweener.
	long double read_chunk(stringstream& ss); //Reads a whole chunk from after start '('
		//to before end ')', or until end, multiple terms and operators.
	long double read_chunk(const string& ss);  //String version of same.
};