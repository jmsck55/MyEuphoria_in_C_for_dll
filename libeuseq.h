
// libeuseq.h

// core function: "modify_seq_range()"

#ifndef __LIBEUSEQ_H__
#define __LIBEUSEQ_H__

#pragma once

// Uncomment next line on 64-bit operating systems:
#define IS_64BIT

#ifndef MY_DLL_API
// Without a DLL or SO, uncomment this line:
#define MY_DLL_API
// When importing from a DLL, uncomment the following line:
// In Microsoft, use this:
//#define MY_DLL_API __declspec(dllimport)
// In Linux, use this:
//#define MY_DLL_API extern
#endif // !MY_DLL_API

#ifdef __cplusplus
extern "C" {
#endif

	
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	

	// Default to IS_32BIT

#ifndef IS_8BIT
#ifndef IS_16BIT
#ifndef IS_64BIT
#define IS_32BIT
#endif // !IS_8BIT
#endif // !IS_16BIT
#endif // !IS_64BIT

	// Standard constants:
	// NOTE: Uncomment this line for old compilers:
	//#define NULL ((void *)0)
#define FALSE 0
#define TRUE 1
	// Written this way for any compiler, some dont have size_t
#define POINTER_SIZE sizeof(void *)
	// Use uinteger instead of size_t
	//#define size_t uinteger

	// MyEuphoria_arch:
	typedef enum {
		// Now, it is mathematical:
		out_dated, // 0, for versions before version 0.2b
		is_8_bit, // 1, reserved, if it is 8-bit
		is_16_bit, // 2, for 16-bit compilers
		is_32_bit, // 3, originally designed for 32-bit
		is_64_bit, // 4, for 64-bit
		is_128_bit // 5, for future versions, if it can be compiled for 128-bit
				   //is_256_bit, // reserved
				   //is_512_bit // reserved
	} *myarch_ptr, myarch;

#ifdef IS_64BIT
	// For 64-bit systems:
	//#pragma pack(8)
#define MyEuphoria_arch is_64_bit
	typedef long long integer;
	typedef unsigned long long uinteger;
#define bool long long
	typedef double short_atom; // In Microsoft, use "short_atom" instead of "atom"
	typedef long double atom; // In Microsoft, "atom" and "short_atom" are the same thing
#ifdef HAS_128BIT
#ifdef IS_IA64
	typedef _Quad long_atom;
#else
	typedef __float128 long_atom;
#endif // IS_IA64
	typedef __int128 longint;
	typedef unsigned __int128 ulongint;
#endif // HAS_128BIT
#endif // IS_64BIT

#ifdef IS_32BIT
	// For 32-bit systems:
	//#pragma pack(4)
#define MyEuphoria_arch is_32_bit
	typedef int integer;
	typedef unsigned int uinteger;
#define bool int
	typedef float short_atom;
	typedef double atom;
	typedef long double long_atom;
	typedef long long longint;
	typedef unsigned long long ulongint;
#endif // IS_32BIT

#ifdef IS_16BIT
	// For 16-bit systems:
	//#pragma pack(2)
#define MyEuphoria_arch is_16_bit
	typedef short integer;
	typedef unsigned short uinteger;
#define bool short;
	typedef short short_atom;
	typedef float atom;
	typedef double long_atom;
	typedef int longint;
	typedef unsigned int ulongint;
#endif // IS_16BIT

#ifdef IS_8BIT
	// For 8-bit systems:
	//#pragma pack(1)
#define MyEuphoria_arch is_8_bit
	typedef char integer;
	typedef unsigned char uinteger;
#define bool char;
	typedef char short_atom;
	typedef char atom;
	typedef float long_atom;
	typedef short longint;
	typedef unsigned short ulongint;
#endif // IS_8BIT

#define IS_EQUAL_DBL (sizeof(atom) == sizeof(short_atom))


	typedef enum {
		SEQ, // 0
		INTEGER, // 1
		FLOAT, // 2
		DOUBLE_PTR, // 3
		DUP, // 4
		REP, // 5
		UBINARY_DATA, // 6
		OTHER // 7 reserved for a future version
	} *is_type_ptr, is_type;

	// The data type "struct obj" or "obj_t" is the size of two (2) pointers

	typedef struct obj {
		// first pointer:
		union {
			// flag, arch, and extended
			struct {
				// first byte:
				struct {
					unsigned char flag : 3; // builtin data types
					unsigned char is_8bit : 1; // if it is 8-bit data type
					unsigned char arch : 4; // extended data types if it is 8-bit
				};
				// Use these variables for extended data types:
				// second byte:
#ifndef IS_8BIT
#ifdef IS_16BIT
				unsigned char extended;
#else
				unsigned char extended8;
#ifdef IS_32BIT
				// third and fourth byte:
				unsigned short extended; // for 32-bit
#else
				// third and fourth byte:
				unsigned short extended16;
#ifdef IS_64BIT
				// fifth to eighth byte:
				unsigned int extended; // for 64-bit
#endif // IS_64BIT
#endif // IS_32BIT
#endif // IS_16BIT
#endif // !IS_8BIT
			};
			integer reserved;
			void * filler; // just to make sure it is the same size as a pointer
		};
		// second pointer
		union {
			integer i;
			void * reserved1;
			short_atom f;
			atom *d_ptr; // pointer to a double
						 // for a future version:
						 // long_atom *atom_ptr; // pointer to a long_atom
						 // longint *i_ptr; // pointer to a long long
			struct obj_count *dup;
			struct obj_count *rep;
			struct seq *s; // linked list of objects
			struct obj_size *ubin; // binary data type, a string of bytes (use "data")
		};
	} obj_t, *pobj_t;


	typedef struct seq {
		integer length; // 0 for no length, normally 1 to number of elements in sequence
		integer position; // zero when first initialized, normally 1 to length, -1 if it is an array
		struct element *first;
		struct element *last;
		union {
			struct element *ptr; // last accessed element
			struct obj **a; // an array of pointers to "obj"s
		};
	} seq_t, *pseq_t;


	typedef struct element {
		struct obj *p;
		struct element *prev;
		struct element *next;
	} element_t, *pelement_t;


	typedef struct obj_count {
		integer count;
		struct obj *p;
	} dup_t, *pdup_t, rep_t, *prep_t;


	typedef struct obj_size {
		integer size;
		unsigned char *bytes;
	} ubin_t, *pubin_t;


#define IS_OBJ(T) (T != NULL)
#define IS_DUP(T) (T->flag == DUP)
#define IS_REP(T) (T->flag == REP)
#define IS_SEQ(T) (T->flag == SEQ)
#define IS_INTEGER(T) (T->flag == INTEGER)
#define IS_FLOAT(T) (T->flag == FLOAT)
#define IS_DOUBLE_PTR(T) (T->flag == DOUBLE_PTR)
#define IS_UBINARY(T) (T->flag == UBINARY_DATA)


	// exported functions prototypes go here:

	//MY_DLL_API void myfun ( int * a);

	MY_DLL_API void lib_atexit_func(void);
	MY_DLL_API void delete_objs(integer count, pobj_t *ptr);
	MY_DLL_API void delete_obj(pobj_t ptr);

	MY_DLL_API pobj_t new_obj(pobj_t from); // copy, creates "dup"
	MY_DLL_API pobj_t new_int(const integer i);
	MY_DLL_API pobj_t new_float(const short_atom * f); // Use this on Microsoft for 64-bit
	MY_DLL_API pobj_t new_double(const atom * d_ptr);
	MY_DLL_API pobj_t new_ubinary(const unsigned char *ustr, integer size);

	MY_DLL_API pobj_t repeat_obj(pobj_t ob, integer count);
	MY_DLL_API pobj_t clone_obj(pobj_t from, bool is_writeable);
/*
	MY_DLL_API pobj_t new_objs(pobj_t * from, integer num); // copy, creates "dup"
	MY_DLL_API pobj_t new_ints(const integer * i, integer num);
	MY_DLL_API pobj_t new_floats(const short_atom ** f, integer num); // Use this on Microsoft for 64-bit
	MY_DLL_API pobj_t new_doubles(const atom ** d_ptr, integer num);

	typedef struct {
		unsigned char * ptr;
		integer size;
	} *pchar_and_size, char_and_size;

	MY_DLL_API pobj_t new_ubins(pchar_and_size * ptr, integer num);
*/
	MY_DLL_API void set_copy_ubin(bool condition);

	MY_DLL_API bool is_obj(pobj_t obj);
	MY_DLL_API bool is_dup(pobj_t obj);
	MY_DLL_API bool is_rep(pobj_t obj);
	MY_DLL_API bool is_seq(pobj_t obj);
	MY_DLL_API bool is_integer(pobj_t obj);
	MY_DLL_API bool is_short_atom(pobj_t obj);
	MY_DLL_API bool is_atom_ptr(pobj_t obj);
	MY_DLL_API bool is_ubinary(pobj_t obj);

	MY_DLL_API integer get_length(pobj_t obj);

	MY_DLL_API pobj_t c_seq_at(pobj_t ret, integer index, bool is_force_array); // read-only
	MY_DLL_API unsigned char * c_ubin_at(pobj_t ret, integer index); // returns a pointer for read/write access

	MY_DLL_API bool c_modify_seq_range(pobj_t ret, integer index_start, integer index_stop, pobj_t orig);

	MY_DLL_API void print_obj(pobj_t obj);


	// Future:

	// compare
	// then merge duplicates (dup)




#ifdef __cplusplus
} // extern "C"
#endif


#endif // __LIBEUSEQ_H__
