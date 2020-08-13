
// Working towards version v1.0

// MyEuphoria_v1_0.cpp

//#include "stdafx.h"

#include <iostream>

#include "libeuseq.h"

int main(void) // (int argc, char* argv[])
{
	// Unit Testing, test all features.

	//lib_init(); // takes the address of a pointer

	// This statement has to be at the beginning of your program:
	if (atexit(lib_atexit_func) != 0) // calls function when exit() is called
	{
		// You can put a custom error message here:
		printf("Error, Unable to register \"atexit\" function\n");
		system("pause");
		return 0;
	}

	pobj_t test, test1, test2, test3;

	/*
	new_obj
	new_int
	new_float
	new_double
	new_ubinary
	repeat_obj
	clone_obj

	// read-only
	c_seq_at() // read-only
	c_ubin_at() // read or write, depending on preference
	// write-only
	c_modify_seq_range()
	*/



	char str[] = "Hi world";
	test = new_ubinary((unsigned char *)str, strlen(str) + 1);

	std::cout << "Program output:\n";

	test1 = new_obj(NULL); //repeat_obj(NULL, 0);

	//delete_obj(test2);
	test2 = new_int(1);
	if (FALSE == c_modify_seq_range(test1, -1, -2, test2)) {
		printf("false\n");
	}
	delete_obj(test2);
	test2 = new_int(2);
	if (FALSE == c_modify_seq_range(test1, -1, -2, test2)) {
		printf("false\n");
	}
	delete_obj(test2);
	test2 = new_int(3);
	if (FALSE == c_modify_seq_range(test1, -1, -2, test2)) {
		printf("false\n");
	}
	delete_obj(test2);

	//test2 = new_int(999);
	//test1 = repeat_obj(test2, 4);
	//delete_obj(test2);
	test2 = new_obj(test1);
	test3 = new_int(44);

	if (FALSE == c_modify_seq_range(test2, -1, -2, test1)) { // append
		printf("false\n");
	}
	if (FALSE == c_modify_seq_range(test2, 0, -1, test1)) { // prepend
		printf("false\n");
	}
	print_obj(test2);
	std::cout << std::endl;

	//here

	delete_obj(test); // always free top-level objects before bottom-level objects
	delete_obj(test1);
	delete_obj(test2);
	delete_obj(test3);

	// This statemnet has to be at the end of your program, or before it returns:
	lib_atexit_func(); // frees all remaining top-level objects

					   //system("pause");

	return 0;
}
