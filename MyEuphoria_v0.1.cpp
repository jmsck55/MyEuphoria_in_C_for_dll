
// Working towards version v1.0

// MyEuphoria_v1_0.cpp

//#include "stdafx.h"


#include <iostream>
#include <string>
#include <vector>

#include <math.h>


#include "libeuseq.h"

integer i, pos, len, start, stop;
char ch;
atom d;
std::string name, st;
std::string booltext[2] = { "false","true" };
std::vector< std::string > names;
std::vector< pobj_t > vars;

integer select(void) {
	// modifies variables: name, ch, pos, len, i, names, vars
	std::cout << "Enter a name: ";
	std::cin >> name;
	std::cout << "You typed: " << name << std::endl;
	std::cout << "Are you sure? [yn] ";
	std::cin >> st;
	ch = st[0];
	if (ch == 'y') {
		len = (integer)names.size();
		for (i = 0; i < len; i++) {
			if (names[i].compare(name) == 0) {
				return i;
				break;
			}
		}
		names.push_back(name);
		vars.push_back(NULL);
		return names.size() - 1;
	}
	return -1;
}

int main(void) // (int argc, char* argv[])
{
	// Unit Testing, test all features.

	// For "C++" programs, This statement has to be at the beginning of your "C++" program:
	if (atexit(lib_atexit_func) != 0) // calls function when exit() is called
	{
		// You can put a custom error message here:
		printf("Error, Unable to register \"atexit\" function\n");
		system("pause");
		return 1; // EXIT_FAILURE
	}

	// Blurb:
	std::cout << "Software: MyEuphoria, first demo" << std::endl;
	integer bits1, bits2;
	bits1 = ((integer)pow(2, MyEuphoria_arch - 1)) * 8;
	bits2 = POINTER_SIZE * 8;
	std::cout << "MyEuphoria_arch is: " << "pow(2," << MyEuphoria_arch << "-1)*8 = " << bits1 << "-bit" << std::endl;
	std::cout << "Pointer size is: " << POINTER_SIZE << " bytes = " << bits2 << "-bit" << std::endl;
	std::cout << "Are MyEuphoria_arch and Pointer size the same bits? " << booltext[bits1 == bits2] << std::endl;
	std::cout << "Are short_atom and atom equal? " << booltext[IS_EQUAL_DBL] << std::endl;

	// main while loop:
	pos = 0;
	name = "default";
	names.push_back(name);
	vars.push_back(NULL);
	bool run = true;
	while (run) {
		std::cout << std::endl;
		std::cout << "NOTE: This program is Not fully error-checked. Type in correct values." << std::endl;
		std::cout << "Current name: " << names[pos] << std::endl;
		std::cout << "There are now " << names.size() << " variable names." << std::endl;
		std::cout << "Commands:" << std::endl;
		std::cout << "(q)uit (s)elect (i)nteger (d)ouble (r)epeat (p)rint (n)ew_from (m)odify" << std::endl;
		std::cin >> st;
		ch = st[0];
		switch (ch) {
		case 'q': // quit
			run = false;
			break;
		case 'p': // print to screen
			print_obj(vars[pos]);
			std::cout << std::endl;
			break;
		case 's': // select, prompt for name
			i = select();
			if (i >= 0) {
				pos = i;
			}
			break;
		case 'i': // input an integer
			std::cout << "Type in an integer: " << std::endl;
			std::cin >> st;
			sscanf_s(st.c_str(), "%lli", &i);
			std::cout << "You typed: " << i << std::endl;
			std::cout << "Is this correct? [yn] ";
			std::cin >> st;
			ch = st[0];
			if (ch == 'y') {
				if (vars[pos] != NULL) {
					delete_obj(vars[pos]);
				}
				vars[pos] = new_int(i);
			}
			break;
		case 'd': // input a double
			std::cout << "Type in a floating point number: " << std::endl;
			std::cin >> st;
			sscanf_s(st.c_str(), "%La", &d);
			std::cout << "You typed: " << d << std::endl;
			std::cout << "Is this correct? [yn] ";
			std::cin >> st;
			ch = st[0];
			if (ch == 'y') {
				if (vars[pos] != NULL) {
					delete_obj(vars[pos]);
				}
				vars[pos] = new_double(&d);
			}
			break;
		case 'r': // repeat
			std::cout << "Store repeat into: " << names[pos] << std::endl;
			i = select();
			if (i < 0) {
				break;
			}
			if (i == pos) {
				std::cout << "Error: Not supported yet #01" << std::endl;
				break;
			}
			std::cout << "Enter repeat count as an integer: ";
			std::cin >> st;
			sscanf_s(st.c_str(), "%lli", &len);
			if (len < 0) {
				std::cout << "Error with number: " << len << std::endl;
				break;
			}
			std::cout << "You entered: " << len << ", are you sure? [yn] ";
			std::cin >> st;
			ch = st[0];
			if (ch == 'y') {
				if (vars[pos] != NULL) {
					delete_obj(vars[pos]);
				}
				vars[pos] = repeat_obj(vars[i], len);
			}
			break;
		case 'n': // new from copy or at
			std::cout << "Store new \"at\" into: " << names[pos] << std::endl;
			i = select();
			if (i < 0) {
				break;
			}
			if (i == pos) {
				std::cout << "Error: Not supported yet #01" << std::endl;
				break;
			}
			std::cout << "Enter (-1) for copy, or at position, as an integer: ";
			std::cin >> st;
			sscanf_s(st.c_str(), "%lli", &len);
			if (len < -1) {
				std::cout << "Error with number: " << len << std::endl;
				break;
			}
			std::cout << "You entered: " << len << ", are you sure? [yn] ";
			std::cin >> st;
			ch = st[0];
			if (ch == 'y') {
				if (vars[pos] != NULL) {
					delete_obj(vars[pos]);
				}
				if (len == -1) {
					vars[pos] = new_obj(vars[i]);
				}
				else {
					vars[pos] = new_obj(c_seq_at(vars[i], len, FALSE));
				}
			}
			break;
		case 'm': // c_modfiy_seq_range()
			// not done yet
			std::cout << "Modify sequence: " << names[pos] << std::endl;
			i = select();
			if (i < 0) {
				break;
			}
			if (i == pos) {
				std::cout << "Error: Not supported yet #01" << std::endl;
				break;
			}
			std::cout << "Enter range of sequence to modify as two (2) integers, \"Start\" and \"Stop\"" << std::endl;
			std::cout << "Start: ";
			std::cin >> st;
			sscanf_s(st.c_str(), "%lli", &start);
			std::cout << "Stop: ";
			std::cin >> st;
			sscanf_s(st.c_str(), "%lli", &stop);
			std::cout << "You entered: " << start << " to " << stop << ", are you sure? [yn] ";
			std::cin >> st;
			ch = st[0];
			if (ch == 'y') {
				if (c_modify_seq_range(vars[pos], start, stop, vars[i]) == TRUE) {
					std::cout << "Success" << std::endl;
				}
				else {
					std::cout << "Error: There was an error in \"c_modify_seq_range()\"" << std::endl;
				}
			}
			break;
		default:
			run = false;
			break;
		}
	}
	// For "C" like programs, Always call this function before returning to the operating system:
	//lib_atexit_func();
	return 0; // EXIT_SUCCESS
}
