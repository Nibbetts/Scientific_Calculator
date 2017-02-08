/*
A container and handler for user-defined and built-in variables and constants.
*/

#pragma once
#include <vector>
#include <cmath>

using namespace std;

class VariableContainer
{
public:

	VariableContainer();
	~VariableContainer();
	void assign(const string& name, const long double& value);
	long double read(const string& name);
	string get_message() const;
	bool get_quit() const;

protected:

	string message;
	bool quit;
	vector<string> names;
	vector<long double> values;
};