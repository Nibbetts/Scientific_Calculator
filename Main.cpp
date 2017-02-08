/*
Scientific Calculator
////Reads in strings and performs mathematical operations.

Author: Nathan Tibbetts
Started: 5 December 2016
*/

#include <iostream> 
#include "Calculator.h"
//#include <cstdlib>

using namespace std;

int main()
{
	string input;
	bool quit = false;
	long double answer = 0;
	unsigned short int undefined = 0;

	//system("MODE CON COLS=69 LINES=43");

	Calculator* calculator = new Calculator();
	
	if (!quit) 
	{
		calculator->output("\n\t\t\t     SCIENTIFIC CALCULATOR");
		calculator->print_operations();
	}

	while (!quit) 
	{
		cin >> input;
		if (input != "" && calculator->calculate(input)) 
		{
			answer = calculator->get_answer();
			calculator->output(answer);
		}
		cout << endl;
		quit = calculator->get_quit();
	}

	calculator->~Calculator();
	delete calculator; //DELETING TWICE??
	
	system("Pause");
	return 0;
}