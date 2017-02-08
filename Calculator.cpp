#include "Calculator.h"
using namespace std;

///////////////////////////////////////////////////////////////////////////////
////--------------------------////PUBLIC:////------------------------------////
///////////////////////////////////////////////////////////////////////////////

Calculator::Calculator()
{
	keep_log = true;
	answer = 0;
	bad_input = false;
	quit = false;
	is_in_abs = false;
	is_radians = true;

	log_file.open("log.txt");
	if (log_file.fail()) 
	{
		cout << "ERROR: Cannot open log file for reading and writing! No log will be kept."
			<< endl;
		keep_log = false;
	}
	load_settings();

	//if (previous_input.size() == 0) previous_input.push_back("0");
	//if (previous_answer.size() == 0) previous_answer.push_back(0);
	if (variable_names.size() == 0)
	{
		////variable_assign("x", 0); I realized these are unneeded.
		////variable_assign("y", 0); "
		////variable_assign("t", 0); "
		//variable_assign("n", 0); //????!!!
		//variable_assign("i", 0); //?
	}
}

Calculator::~Calculator() {}

bool Calculator::calculate(string input)
{
	stringstream ss;
	char next_ch;

	previous_input.push_back(input);
	ss << input;
	pre_check(ss);

	if (!read_nonreal(ss, false) && !read_assignment_or_boolean(ss, false))
	{
		answer = read_chunk(ss);
	}

	if (bad_input)
	{
		if (previous_answer.size() > 0) answer = previous_answer.size() - 1;
		else answer = 0;
		bad_input = false;
	}
	else previous_answer.push_back(answer);
	return !bad_input;
}

long double Calculator::get_answer() const { return answer; }

bool Calculator::get_quit() const { return quit; }

void Calculator::print_operations()
{
	ifstream in_file;
	ostringstream ss;
	char ch;

	in_file.open("Printfunctions.txt");
	if (in_file.fail()) output("ERROR: Could not read from Printfunctions.txt! Skipping it.");
	else 
	{
		while (in_file.get(ch)) ss << ch;
		cout << endl << ss.str() << endl << endl;
	}
}

//Outputs to both the console and a log file.
void Calculator::output(string out)
{
	cout << out << endl;
	if (keep_log) log_add(out);
}

void Calculator::output(long double out) { output(to_string(out)); }

///////////////////////////////////////////////////////////////////////////////
////--------------------------////PROTECTED:////---------------------------////
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////TOOLS//////////////////////////////////

void Calculator::log_add(string out)
{
	if (keep_log) log_file << out;
}

void Calculator::load_settings()
{
	ifstream in_file;
	char input;
	string tag;

	in_file.open("Settings.ini");
	if (in_file.fail()) output("\nAMNESIA: Could not read from Settings.ini! Skipping it.");
	else 
	{
		input = '1';
		in_file >> tag;
		in_file.get(input);
		//if (input == '0') is_radians = false; else is_radians = true;
		is_radians = (!(input == '0'));
		//MORE SETTINGS GO HERE.
		//ANSWERS!
		//PRECISION?
		//KEEP_LOG
	}
	if (in_file.fail()) 
	{
		save_settings();
		output("\nERROR: Settings file is corrupted! It has been repaired or replaced.");
	}
}

void Calculator::save_settings()
{
	ofstream out_file;

	out_file.open("Settings.ini");
	if (out_file.fail()) output("\nERROR: Could not write to Settings.ini! Skipping it.");
	else 
	{
		out_file << "Radians: " << is_radians << endl;
		//MORE SETTINGS GO HERE.
	}
	if (out_file.fail()) output("\nERROR: Settings file is having a conniption!");
}

//Removes all spaces and newline characters and comments from the input,
	//and if the first char is an operator adds "ans" before it.
	//@this is a comment@
void Calculator::pre_check(stringstream& ss)
{
	char next_ch;
	string result;
	bool comment = false;

	while (!ss.fail())
	{
		ss.get(next_ch);
		if (next_ch == '@') comment = !comment; // or just break;
		if (next_ch != ' ' && next_ch != '\n' && !comment) result += next_ch;
	}
	ss.clear();

	//Add "ans" before if starts with an operator.
	if (result[0] == '+' || result[0] == '-' || result[0] == '*' || result[0] == '/'
		|| result[0] == '^' || result[0] == '%' || result[0] == '!') result = "ans" + result;

	ss << result;
}

void Calculator::variable_assign(const string& name, const long double& value)
{
	if (variable_names.size() != variable_values.size())
	{
		bad_input = true;
		quit = true;
		output("FATAL ERROR: Internal variable array sizes are mismatched. Exiting.");
	}
	else
	{
		for (int i = 0; i < variable_names.size(); i++)
		{
			if (name == variable_names[i])
			{
				variable_values[i] = value;
				return;
			}
		}
		variable_names.push_back(name);
		variable_values.push_back(value);
	}
	return;
}

//Checks for &(@) and executes it. Checks () itself because this is a very limited case.
bool Calculator::check_in_between(stringstream& ss)
{
	char next_ch = '\n';
	bool parentheses = false;

	if (!bad_input) 
	{
		ss.get(next_ch);
		if (next_ch == '&') 
		{
			ss.get(next_ch);
			if (next_ch == '(') parentheses = true; else ss.unget(); //USE GET_ARGUMENTS?
			if (!read_nonreal(ss, true) && !read_assignment_or_boolean(ss, true)) 
			{
				bad_input = true;
				output("Expected operator or non-real function in interrim.");
			}
			if (parentheses) 
			{
				ss.get(next_ch);
				if (!ss.fail())
				{
					bad_input = !(next_ch == ')');
					if (bad_input) output("Expected ')' after interrim function.");
				}
				else output("Info: Missing parentheses.");
			}
		}
		else 
		{
			ss.clear();
			ss.unget();
			return false;
		}
	}
	ss.clear();
	if (!bad_input) return true; else return false;
}

//Checks if end of input string, also checks for &(@) and executes it,
	//still returning true if ends after that.
bool Calculator::check_end(stringstream& ss)
{
	char next_ch;

	while (check_in_between(ss));
	ss.get(next_ch);
	if (!ss.fail())
	{
		ss.unget();
		return false;
	}
	else 
	{
		ss.clear();
		return true;
	}
}

//Needs one upper case letter, then lower case letters or underscores until a "(".
//If this fails, then sets bad_input = true and returns a fake.
string Calculator::get_function_name(stringstream& ss)
{
	char next_ch;
	string funct_name;

	ss.get(next_ch);
	if (!ss.fail())
	{
		funct_name += next_ch;
		if (isupper(next_ch))
		{
			while (ss.get(next_ch) && (islower(next_ch) || next_ch == '_'))
			{
				funct_name += next_ch;
			}
			if (next_ch == '(')
			{
				return funct_name;
			}
			else
			{
				bad_input = true;
				if (isupper(next_ch)) output("Only lower-case letters and '_' allowed after start of function name.");
				else output("Expected '(' after function name.");
				return "Fake";
			}
		}
		else
		{
			bad_input = true;
			output("ERROR: Unexpected function call on non-uppercase character.");
			return "Fake";
		}
	}
	else return "Fake";
}

//All lower case. Also use for constants.
string Calculator::get_variable_name(stringstream& ss)
{
	char next_ch;
	string name;

	while (ss.get(next_ch) && islower(next_ch)) name += next_ch;
	if (!ss.fail()) ss.unget();
	if (name == "")
	{
		if (next_ch != 'G')
		{
			bad_input = true;
			output("ERROR: Expected variable or constant name.");
			name = "fake";
		}
		else name = "G";
	}
	return name;
}

/*
Collects the inside of a function's parentheses,
	commas included. Use after first parenthese; discards the end parenthese, if found.
	Can return an empty vector. Counts up and down internal parentheses until right.
*/
vector<string> Calculator::get_arguments(stringstream& ss)
{
	vector<string> args;
	short int parentheses = 0;
	short int current_arg = 0;
	char next_ch;

	ss.get(next_ch);
	while (!ss.fail() && !(parentheses == 0 && next_ch == ')'))
	{
		if (next_ch == ',')
		{
			current_arg++;
			args.push_back("");
		}
		else ////////PROBLEMS!!!!!!!
		{
			args[current_arg] += next_ch;
			if (next_ch == '(') parentheses++;
			if (next_ch == ')') parentheses--;
		}
		ss.get(next_ch);
	}

	return args;
}

//Returns whether the number is an integer.
bool Calculator::is_integer(const long double& value) const
{
	return (value == static_cast<long int>(value));
}

long int Calculator::int_factorial(long int value) const
{
	if (value <= 1) return 1;
	else return value * int_factorial(value - 1);
}

long double Calculator::compute_factorial(const long double& value)
{
	if (!is_integer(value) || value < 0)
	{
		output("Non-integer and negative factorials not yet implemented.");
		bad_input = true;
		return value;
		//GAMMA FUNCTION GOES HERE
	}
	else
	{
		if (value <= 1) return 1;
		else return int_factorial(static_cast<long int>(value));
	}
}

//////////////////////////////SUB-NUMBER LEVEL//////////////////////////////////////

//Actual number reading, with decimal. Doesn't include negatives (two levels up).
long double Calculator::get_number(stringstream& ss)
{
	bool decimal = false;
	string number;
	char next_ch;

	check_in_between(ss);
	if (!bad_input)
	{
		while (ss.get(next_ch) && (isdigit(next_ch) || next_ch == '.'))
		{
			if (next_ch == '.')
			{
				if (decimal)
				{
					bad_input = true;
					output("There was more than one decimal in that number.");
					return 1;
				}
				else decimal = true;
			}
			number += next_ch;
		}
		if (!ss.fail()) ss.unget();
	}
	return (stold(number));
}

//Read real-valued function.
long double Calculator::read_function(stringstream& ss)
{
	const unsigned short int NUM_FUNCTIONS = 38;
	const string FUNCTIONS[NUM_FUNCTIONS] =
	{
		"Sin",				//0
		"Cos",				//1
		"Tan",				//2
		"Csc",				//3
		"Sec",				//4
		"Cot",				//5
		"Arcsin",			//6
		"Arccos",			//7
		"Arctan",			//8
		"Arccsc",			//9
		"Arcsec",			//10
		"Arccot",			//11
		"Sinh",				//12
		"Cosh",				//13
		"Tanh",				//14
		"Arcsinh",			//15
		"Arccosh",			//16
		"Arctanh",			//17
		"Abs",				//18
		"Sqrt"				//19
		"Degtorad",			//20
		"Radtodeg",			//21
		"Trunc",			//22
		"Ln",				//23
		"Logn",				//24
		"Ans",				//25
		"Timetocompute",	//26
		"Round",			//27
		"Convert",			//28
		"Lastinput",		//29
		"Seq",				//30
		"Rand",				//31
		"Randint",			//32
		"Sum",				//33
		"Remem",			//34
		"Bintodec",			//35
		"Hextodec",			//36
		"Nroot"				//37
	};
	const unsigned short int NUM_ARGS[NUM_FUNCTIONS] =
	{
		1,					//0
		1,					//1
		1,					//2
		1,					//3
		1,					//4
		1,					//5
		1,					//6
		1,					//7
		1,					//8
		1,					//9
		1,					//10
		1,					//11
		1,					//12
		1,					//13
		1,					//14
		1,					//15
		1,					//16
		1,					//17
		1,					//18
		1,					//19
		1,					//20
		1,					//21
		1,					//22
		1,					//23
		2,					//24
		1,					//25
		1,					//26
		3,					//27
		3,					//28
		0,					//29
		3,					//30
		0,					//31
		1,					//32
		3,					//33
		0,					//34
		1,					//35
		1,					//36
		2					//37
	};

	string name = get_function_name(ss);
	vector<string> args;
	unsigned short int index = 0;
	long double result = 1;

	if (!bad_input) {
		for (index = 0; index < NUM_FUNCTIONS; index++)
		{
			if (name == FUNCTIONS[index]) break;
		}
		if (index == NUM_FUNCTIONS)
		{
			bad_input = true;
			output("Unknown function name: \"" + name + "\".");
		}
		else
		{
			args = get_arguments(ss);
			if (args.size() != NUM_ARGS[index])
			{
				bad_input = true;
				output("Unexpected number of arguments to function \"" + name + "\".");
			}
			if (!bad_input) switch (index)
			{

			case 0: result = sin(read_chunk(args[0])); break; //Sin
			case 1: result = cos(read_chunk(args[0])); break; //Cos
			case 2: result = tan(read_chunk(args[0])); break; //Tan
			case 3: result = 1.0 / sin(read_chunk(args[0])); break; //Csc
			case 4: result = 1.0 / cos(read_chunk(args[0])); break; //Sec
			case 5: result = 1.0 / tan(read_chunk(args[0])); break; //Cot
			case 6: result = asin(read_chunk(args[0])); break; //Arcsin
			case 7: result = acos(read_chunk(args[0])); break; //Arccos
			case 8: result = atan(read_chunk(args[0])); break; //Arctan
			case 9: result = asin(1.0 / read_chunk(args[0])); break; //Arccsc
			case 10: result = acos(1.0 / read_chunk(args[0])); break; //Arcsec
			case 11: result = atan(1.0 / read_chunk(args[0])); break; //Arccot
			case 12: result = sinh(read_chunk(args[0])); break; //Sinh
			case 13: result = cosh(read_chunk(args[0])); break; //Cosh
			case 14: result = tanh(read_chunk(args[0])); break; //Tanh
			case 15: result = asinh(read_chunk(args[0])); break; //Arcsinh
			case 16: result = acosh(read_chunk(args[0])); break; //Arccosh
			case 17: result = atanh(read_chunk(args[0])); break; //Arctanh

			case 18: result = abs(read_chunk(args[0])); break; //Abs
			case 19: result = sqrt(read_chunk(args[0])); break; //Sqrt
			case 20: result = (read_chunk(args[0]) * CONSTANT_VALUES[0] / 180.0); break; //Degtorad
			case 21: result = (read_chunk(args[0]) * 180.0 / CONSTANT_VALUES[0]); break; //Radtodeg
			case 22: result = static_cast<long int> (read_chunk(args[0])); break; //Trunc
			case 23: result = log(read_chunk(args[0])); break; //Ln
				/////NEGATIVE LOGS!??
			case 24: result = (log(read_chunk(args[1])) / log(read_chunk(args[0]))); break; //Logn
				////TEST THIS ONE, TOO!
			case 25: //Ans
				break;

			case 26: //Timetocompute
				break;

			case 27: //Round
				break;

			case 28: //Convert
				break;

			case 29: //Lastinput
				break;

			case 30: //Seq
				break;

			case 31: //Rand
				break;

			case 32: //Randint
				break;

			case 33: //Sum
				break;

			case 34: //Remem
				break;

			case 35: //Bintodec
				break;

			case 36: //Hextodec
				break;

			case 37: //Nroot
				break;

			default: //This should never happen.
				bad_input = true;
				output("ERROR: Internal real-valued switch statement is having a conniption.");
				break;
			}
		}
	}
	return result;
}

//Includes a search for constants.
const long double* Calculator::read_variable(stringstream& ss)
{
	string name = get_variable_name(ss);

	if (!bad_input)
	{
		for (int i = 0; i < NUM_CONSTANTS; i++)
		{
			if (name == CONSTANT_NAMES[i]) return &CONSTANT_VALUES[i];
		}

		for (int i = 0; i < variable_names.size(); i++)
		{
			if (name == variable_names[i]) return &variable_values[i];
		}
		output("Info: Using uninitialized variable. Value will be set to 0.");
		variable_assign(name, 0); //MAKE OBJECT!!
	}
	return 0;
}

//////////////////////////////REAL-VALUED NUMBER LEVEL//////////////////////////////

//Number literals, (), const, var, real-val func,
	//NOT ! or E because That's one level up.
long double Calculator::read_number(stringstream& ss)
{
	char next_ch;
	long double result = 1;
	bool abs_outside_parentheses = is_in_abs;
	
	ss.get(next_ch);
	if (!ss.fail() && !bad_input)
	{
		if (isdigit(next_ch) || next_ch == '.')
		{
			ss.unget();
			result = get_number(ss);
		}
		else if (next_ch == '(' || next_ch == '[' || next_ch == '{')
		{
			char parentheses_type;
			switch (next_ch)
			{
				case '(': parentheses_type = ')'; break;
				case '[': parentheses_type = ']'; break;
				case '{': parentheses_type = '}'; break;
				default:
					bad_input = true;
					output("ERROR: Tissy fit in parenthetical number reading!");
					break;
			}
			is_in_abs = false;
			result = read_chunk(ss);
			ss.get(next_ch);
			if (!ss.fail() && next_ch != parentheses_type)
			{
				bad_input = true;
				output("Expected matching end parenthese or end of statement.");
			}
			is_in_abs = abs_outside_parentheses;
		}
		else if (next_ch == '|' && !is_in_abs)
		{
			is_in_abs = true;
			result = abs(read_chunk(ss));
			ss.get(next_ch);
			if (!ss.fail() && next_ch != '|')
			{
				bad_input = true;
				output("ERROR: Expected end '|' or end of statement.");
			}
			is_in_abs = false;
		}
		else if (isupper(next_ch))
		{
			ss.unget();
			if (next_ch == 'G') result = *read_variable(ss);
			else result = read_function(ss);
		}
		else if (islower(next_ch))
		{
			ss.unget();
			result = *read_variable(ss);
		}
		else
		{
			bad_input = true;
			output("Unexpected character in number interpretation.");
		}
	}
	return result;
}

//////////////////////////////NUMBER OBJECT LEVEL///////////////////////////////////

//Includes Sci. notation check, and factorial check and computation,
	//Note: Order of operations: -3.2!E-4.5! = -([(3.2!)E(-4.5)]!)
	//Also: "number objeccts" include (), vars, real-valued functions, etc.
long double Calculator::read_number_object(stringstream& ss)
{
	bool minus = false;
	bool minus_magnitude = false;
	char next_ch;
	long double number = 0;
	long double magnitude = 0;

	//Check for -, then number, then !, then E, then -, then number, then ! again.
	while (ss.get(next_ch) && next_ch == '-') minus = !minus;
	if (!ss.fail() && !bad_input)
	{
		ss.unget();
		number = read_number(ss);
		if (!bad_input)
		{
			while (ss.get(next_ch) && next_ch == '!') number = compute_factorial(number);
			if (!ss.fail() && next_ch == 'E')
			{
				//Note: Real-valued functions may not be able to begin with E...
				while (ss.get(next_ch) && next_ch == '-') minus_magnitude = !minus_magnitude;
				if (!ss.fail()) ss.unget();
				magnitude = read_number(ss);
				if (minus_magnitude) magnitude = (0 - magnitude);
				if (!bad_input)
				{
					number *= pow(10, magnitude);
					while (ss.get(next_ch) && next_ch == '!')
					{
						number = compute_factorial(number);
					}
					if (!ss.fail()) ss.unget();
				}
			}
			else if (!ss.fail())
			{
				ss.unget();
				while (check_in_between(ss));
			}
			if (minus) return (0 - number); else return number;
		}
	}
	bad_input = true;
	output("ERROR: Expected number to follow.");
	return 1;
}

//Includes factorial check and computation. Triggers bad_input if not integer.
/*long int Calculator::read_integer_object(stringstream& ss)
{
	long double whole_result = read_number_object(ss);
	long int result = static_cast<long int>(whole_result);
	if (result != whole_result)
	{
		bad_input = true;
		output("Unexpected non-integer in input.");
	}
	return result;
}*/

//////////////////////////////TERM LEVEL/////////////////////////////////////

//Reads, executes, and returns a term.
long double Calculator::read_term(stringstream& ss)
{
	bool check_again = true;
	bool first_time = true;
	long double result = 1;
	char next_ch;

	while (check_again)
	{
		if (ss.get(next_ch))
		{
			if (isdigit(next_ch) || next_ch == '(' || next_ch == '[' || next_ch == '{'
				|| isalpha(next_ch) || (next_ch == '|' && !is_in_abs))
			{
				ss.unget();
				result *= read_number_object(ss);
			}
			else if (next_ch == '*' && !first_time) result *= read_number_object(ss);
			else if (next_ch == '/' && !first_time) result /= read_number_object(ss);
			else if (next_ch == '&') while (check_in_between(ss));
			else if (next_ch == '%' && !first_time)
			{
				long double denom = read_number_object(ss);
				result -= (denom * static_cast<long int>(result / denom));
			}
			else
			{
				ss.unget();
				check_again = false;
				if (first_time)
				{
					bad_input = true;
					output("Expected statement before another operator.");
				}
			}
			first_time = false;
		}
		else
		{
			check_again = false;
			if (first_time)
			{
				bad_input = true;
				output("Expected statement after operator.");
				result = 0;
			}
		}
	}

	return result;
}

/////////////////////////////MULTI-TERM LEVEL////////////////////////////////

//Returns if next piece is a non-real-valued function and executes it,
	//also will return a fail if there is anything after it unless inside an in-betweener.
bool Calculator::read_nonreal(stringstream& ss, bool allow_after)
{
	const unsigned short int NUM_FUNCTIONS = 21;
	const string FUNCTIONS[NUM_FUNCTIONS] = 
	{
		"Mem",				//0
		"Dectobin",			//1
		"Dectohex",			//2
		"Seta_",			//3
		"Intapprox",		//4
		"F",				//5
		"Settings",			//6
		"Printfunctions",	//7
		"Fparam",			//8
		"Setrad",			//9
		"Setdeg",			//10
		"Braille",			//11
		"Dateafter",		//12
		"Essayinfo",		//13
		"Time",				//14
		"Frac",				//15
		"Help",				//16
		"Quit",				//17
		"Encode",			//18
		"Decode",			//19
		"Morse"				//20
	};
	const unsigned short int NUM_ARGS[NUM_FUNCTIONS] =
	{
		1,					//0
		1,					//1
		1,					//2
		2,					//3
		5,					//4
		4,					//5
		0,					//6
		0,					//7
		5,					//8
		0,					//9
		0,					//10
		1,					//11
		6,					//12
		1,					//13
		0,					//14
		1,					//15
		0,					//16
		0,					//17
		2,					//18
		2,					//19
		1					//20
	};

	string name;
	vector<string> args;
	unsigned short int index;
	char next_ch;

	if (!bad_input && ss.get(next_ch) && isupper(next_ch)) {
		ss.unget();
		name = get_function_name(ss);
		for (index = 0; index < NUM_FUNCTIONS; index++) 
		{
			if (name == FUNCTIONS[index]) break;
		}
		if (index == NUM_FUNCTIONS) return 0;
		else 
		{
			args = get_arguments(ss);
			if (args.size() != NUM_ARGS[index])
			{
				bad_input = true;
				output("Unexpected number of parameters to function \"" + name + "\".");
			}
			if (allow_after) 
			{
				bad_input = !check_end(ss);
				if (bad_input) output("Unexpected continuation after non-real function.");
			}
			else check_end(ss);
			if (!bad_input) switch (index) 
			{

				case 0: //Mem
					break;

				case 1: //Dectobin
					break;

				case 2: //Dectohex
					break;

				case 3: //Seta_
					break;

				case 4: //Intapprox
					break;

				case 5: //F
					break;

				case 6: //Settings
					break;

				case 7: print_operations(); break; //Printfunctions

				case 8: //Fparam
					break;

				case 9: //Setrad
					is_radians = true;
					save_settings();
					break;

				case 10: //Setdeg
					is_radians = false;
					save_settings();
					break;

				case 11: //Braille
					break;

				case 12: //Dateafter
					break;

				case 13: //Essayinfo
					break;

				case 14: //Time
					break;

				case 15: //Frac
					break;

				case 16: //Help
					break;

				case 17: quit = true; break; //Quit

				case 18: //Encode
					break;

				case 19: //Decode
					break;

				case 20: //Morse
					break;

				default: //This should never happen.
					bad_input = true;
					output("ERROR: Internal non-real switch statement is having a conniption.");
					break;
			}
		}
		return 1;
	}
}

/*
Returns if next piece is an assignment or a boolean and executes it,
also will return bad input if there is anything after unless inside an in-betweener,
and won't change answer if in an in-betweener.
*/
bool Calculator::read_assignment_or_boolean(stringstream& ss, bool allow_after)
{
	//FILL!!
	return 0;
}

//Reads a whole chunk from after start '(' to before end ')',
	//or until end, multiple terms and operators.
long double Calculator::read_chunk(stringstream& ss)
{
	char next_ch;
	long double result = 0;

	if (!bad_input && !quit)
	{
		if (ss.str() == "")
		{
			bad_input = true;
			output("Expected a statement."); //Should only occur if user puts empty parentheses.
		}
		else
		{
			result = read_term(ss);
			while (ss.get(next_ch) && (next_ch == '+' || next_ch == '-'))
			{
				if (next_ch == '+') result += read_term(ss);
				if (next_ch == '-') result -= read_term(ss);
			}
			if (!ss.fail())
			{
				ss.unget();
				if (next_ch != ')' && next_ch != ']' && next_ch != '}' && next_ch != '|')
				{
					bad_input = true;
					output("ERROR: Unexpected character in chunk reading: " + next_ch);
				}
			}
		}
	}
	return result;
}

//String version of normal read_chunk, used for arguments
long double Calculator::read_chunk(const string& str)
{
	stringstream ss;
	ss << str;
	return read_chunk(ss);
}