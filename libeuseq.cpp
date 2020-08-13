
// libeuseq.cpp

//#include "stdafx.h"

// When building a DLL, uncomment the following line
//#define MY_DLL_API __declspec(dllexport)

#include "libeuseq.h"

#ifdef __cplusplus
extern "C" {
#endif

	bool is_copy_ubin = TRUE;

	// local function prototypes:

	pobj_t local_new_obj(pobj_t from); // copy, creates "dup"
	pobj_t local_new_int(const integer i);
	pobj_t local_new_float(const short_atom * f); // Use this on Microsoft for 64-bit
	pobj_t local_new_double(const atom * d_ptr);
	pobj_t local_new_ubinary(const unsigned char *ustr, integer size);
	pobj_t local_repeat_obj(pobj_t ob, integer count);
	pobj_t local_clone_obj(pobj_t from, bool is_writeable);

	void register_obj(pobj_t obj);
	void delete_seq(pseq_t s);
	void local_delete_obj(pobj_t obj);

	pobj_t allocate_obj(void * val1, integer flags, integer extended);
	pdup_t allocate_dup(pobj_t p, integer count);
	prep_t allocate_rep(pobj_t p, integer count);
	pubin_t allocate_ubinary(unsigned char *ustr, integer size);

	inline integer c_get_index(integer index, integer len);
	inline pseq_t new_empty_seq(void);
	pseq_t new_seq_one_element(pobj_t ob);

	inline pobj_t get_non_dup(pobj_t obj);
	pseq_t expand_repeat(prep_t r, bool is_writeable); // dont use frequently

	bool flush_to_array(pseq_t s);
	pseq_t concat_seq(pseq_t * from, integer num); // takes an array of "pseq_t" pointers, of length "num"


	// element access:

	//#ifdef USE_C_OFFSETS
	pelement_t c_element_seek(pseq_t myseq, integer index);

	bool c_delete_seq_range(pseq_t s, integer index_start, integer index_stop);

	// write-only
	bool c_insert_slice(pseq_t s, integer index, pobj_t orig);
	pseq_t c_new_seq(pseq_t s_from, integer index, integer range_len);
	pseq_t c_clone_seq_modify_range(const pseq_t s, integer index_start, integer index_stop, pobj_t orig);
	pseq_t c_expand_repeat_modify_range(prep_t rep, integer index_start, integer index_stop, pobj_t orig);
	//#endif // USE_C_OFFSETS



	// exported, imported functions definitions go here:

	//void myfun(int * a) {
	//    *a = - *a;
	//}


	typedef struct obj_next {
		struct obj *p;
		struct obj_next *next;
	} obj_next_t, *pobj_next_t;

	pobj_next_t all_objects = NULL; // all top-level objects

									// Exported function:
	void lib_atexit_func(void) {
		//done.
		// cleanup_obj();
		pobj_next_t tmp;
		while (all_objects != NULL) {
			local_delete_obj(all_objects->p);
			tmp = all_objects;
			all_objects = tmp->next;
			free(tmp);
		}
	}

	void delete_objs(integer count, pobj_t *ptr) {
		//done.
		pobj_next_t objs, prev;
		integer i;
		objs = all_objects;
		prev = NULL;
		while (objs != NULL) {
			for (i = 0; i < count; i++) {
				if (ptr[i] == objs->p) {
					// remove and free "objs", and "save objs->next"
					local_delete_obj(objs->p);
					if (prev == NULL) {
						all_objects = objs->next;
					}
					else {
						prev->next = objs->next;
					}
					free(objs);
					objs = prev;
					break;
				}
			}
			if (objs == NULL) {
				break;
			}
			prev = objs;
			objs = objs->next;
		}
	}
	void delete_obj(pobj_t ptr) {
		delete_objs(1, &ptr);
	}

	// Global:
	pobj_t new_obj(pobj_t from) {
		//done.
		pobj_t ret;
		ret = local_new_obj(from);
		register_obj(ret);
		return ret;
	}
	pobj_t local_new_obj(pobj_t from) {
		//done.
		// could make a duplicate, or allocate an integer or float
		pobj_t obj;
		if (from == NULL) {
			obj = allocate_obj(NULL, 0, 0);
			return obj;
		}
		obj = allocate_obj(from->reserved1, from->flag, 0); // zero (0) on purpose, if needed, set after "local_new_obj()" returns
		switch (from->flag) {
			// it falls-thru unless there is the "break" keyword
		case DUP:
			// uses fall-thru
			from->dup->count++;
		case INTEGER:
			// uses fall-thru
		case FLOAT:
			// uses fall-thru
			break;
		default:
			// any pointer:
			// make it into a "shadow copy" or duplicate
			from->dup = allocate_dup(obj, 2); // obj is stored into dup
			from->flag = DUP;
			obj = allocate_obj(from->dup, DUP, 0);
			break;
		}
		return obj;
	}
	pobj_t new_int(const integer i) {
		//done.
		pobj_t ret;
		ret = local_new_int(i);
		register_obj(ret);
		return ret;
	}
	pobj_t local_new_int(const integer i) {
		//done.
		pobj_t obj;
		obj = allocate_obj((void *)i, INTEGER, 0);
		return obj;
	}
	pobj_t new_float(const short_atom * f) {
		//done.
		pobj_t ret;
		ret = local_new_float(f);
		register_obj(ret);
		return ret;
	}
	pobj_t local_new_float(const short_atom * f) {
		//done.
		pobj_t obj;
		obj = allocate_obj(0, FLOAT, 0);
		if (obj == NULL) {
			return NULL;
		}
		memcpy(obj + POINTER_SIZE, f, sizeof(short_atom)); // pointer size and short_atom size are the same
		return obj;
	}
	pobj_t new_double(const atom * d_ptr) {
		//done.
		pobj_t ret;
		ret = local_new_double(d_ptr);
		register_obj(ret);
		return ret;
	}
	pobj_t local_new_double(const atom * d_ptr) {
		//done.
		pobj_t obj;
		atom * d_dst;
		d_dst = (atom *)malloc(sizeof(atom));
		if (d_dst == NULL) {
			return NULL;
		}
		memcpy(d_dst, d_ptr, sizeof(atom));
		obj = allocate_obj(d_dst, DOUBLE_PTR, 0);
		return obj;
	}
	pobj_t new_ubinary(const unsigned char *ustr, integer size) {
		//done.
		pobj_t ret;
		ret = local_new_ubinary(ustr, size);
		register_obj(ret);
		return ret;
	}
	pobj_t local_new_ubinary(const unsigned char *ustr, integer size) {
		//done.
		// It needs to copy the data so it can deallocate it dynamically.
		pobj_t obj;
		pubin_t s;
		unsigned char *dst;
		dst = (unsigned char *)malloc(size);
		if (dst == NULL) {
			return NULL;
		}
		memcpy(dst, ustr, size);
		s = allocate_ubinary(dst, size);
		obj = allocate_obj(s, UBINARY_DATA, 0);
		return obj;
	}
	/*
	pobj_t new_objs(pobj_t * from, integer num) { // copy, creates "dup"
	//here
	pseq_t s;
	integer i;
	if (num < 0) {
	return NULL;
	}
	s = new_empty_seq();
	for (i = 0; i < num; i++) {
	if (c_insert_slice(s, i, from[i]) == FALSE) {
	// wasn't able to insert
	}
	}
	ret = allocate_obj(s, SEQ, 0);

	}
	pobj_t new_ints(const integer * i, integer num) {

	}
	pobj_t new_floats(const short_atom ** f, integer num) { // Use this on Microsoft for 64-bit

	}
	pobj_t new_doubles(const atom ** d_ptr, integer num) {

	}
	pobj_t new_ubins(pchar_and_size * ptr, integer num) { // new sequence of ubins

	}
	*/

	pobj_t repeat_obj(pobj_t ob, integer count) {
		//done.
		pobj_t ret;
		ret = local_repeat_obj(ob, count);
		register_obj(ret);
		return ret;
	}
	pobj_t local_repeat_obj(pobj_t ob, integer count) {
		//done.
		pobj_t obj, p;
		prep_t rep;
		if (count > 0) {
			p = local_new_obj(ob); // this function call could make a duplicate
		}
		else {
			p = NULL;
		}
		rep = allocate_rep(p, count);
		obj = allocate_obj(rep, REP, 0);
		return obj;
	}

	void set_copy_ubin(bool condition) {
		//done.
		is_copy_ubin = condition;
	}

	pobj_t clone_obj(pobj_t from, bool is_writeable) {
		//done.
		pobj_t ret;
		ret = local_clone_obj(from, is_writeable);
		register_obj(ret);
		return ret;
	}
	pobj_t local_clone_obj(pobj_t from, bool is_writeable) {
		//done.
		// Create a pseudo "Clone" (carbon-copy) for reading and writing
		// The resulting object should have no duplicates.
		// For speed, this function should be used "only" for small sections of data, not the entire data stream.
		pobj_t ob, obj;
		pseq_t s;
		pelement_t ele, ele_from, prev;
		integer i, len;
		if (from == NULL) {
			return NULL;
		}
		ob = get_non_dup(from);
		switch (ob->flag)
		{
		case INTEGER:
			obj = local_new_int(ob->i);
			return obj;
			break;
		case FLOAT:
			obj = local_new_float(&ob->f); // takes addres of short_atom
			return obj;
			break;
		case DOUBLE_PTR:
			obj = local_new_double(ob->d_ptr);
			return obj;
			break;
		case UBINARY_DATA:
			if ((is_copy_ubin == TRUE) || (is_writeable == TRUE)) {
				obj = local_new_ubinary(ob->ubin->bytes, ob->ubin->size); // copies the entire thing
			}
			else {
				obj = local_new_obj(from); // pass the original "dup" to save on memory, it is only read-able
			}
			return obj;
			break;
		case REP:
			if (is_writeable == TRUE) {
				s = expand_repeat(ob->rep, is_writeable);
				obj = allocate_obj(s, SEQ, 0);
			}
			else {
				// this function call could make a duplicate
				obj = local_repeat_obj(ob->rep->p, ob->rep->count);
			}
			return obj;
			break;
		case SEQ:
			s = new_empty_seq(); // allocates an empty sequence
			if (s == NULL) {
				return NULL;
			}
			len = ob->s->length;
			if (len > 0) {
				ele = (pelement_t)malloc(sizeof(element_t));
				if (ele == NULL) {
					free(s);
					return NULL;
				}
				s->length = len;
				s->first = ele;
				ele_from = ob->s->first;
				if ((is_writeable == TRUE) || (ob != from)) { // if (ob != from), then it is a "dup", make a copy
					ele->p = local_clone_obj(ele_from->p, TRUE); // copies the entire thing
				}
				else {
					// this function call could make a duplicate
					ele->p = local_new_obj(ele_from->p); // read-only
				}
				ele->prev = NULL;
				for (i = 1; i < len; i++) {
					prev = ele;
					ele = (pelement_t)malloc(sizeof(element_t));
					if (ele == NULL) {
						return NULL;
					}
					ele_from = ele_from->next;
					if ((is_writeable == TRUE) || (ob != from)) { // if (ob != from), then it is a "dup", make a copy
						ele->p = local_clone_obj(ele_from->p, TRUE); // copies the entire thing
					}
					else {
						// this function call could make a duplicate
						ele->p = local_new_obj(ele_from->p); // read-only
					}
					ele->prev = prev;
					prev->next = ele;
				}
				ele->next = NULL;
				s->last = ele;
			}
			obj = allocate_obj(s, SEQ, 0);
			return obj;
			break;
		default:
			//obj = NULL;
			//return obj;
			break;
		}
		return NULL;
	}

	bool is_obj(pobj_t obj) {
		//done.
		return IS_OBJ(obj);
	}
	bool is_dup(pobj_t obj) {
		//done.
		if (IS_OBJ(obj)) {
			if (IS_DUP(obj)) {
				return TRUE;
			}
		}
		return FALSE;
	}
	bool is_rep(pobj_t obj) {
		//done.
		if (IS_OBJ(obj)) {
			if (IS_REP(obj)) {
				return TRUE;
			}
		}
		return FALSE;
	}
	bool is_seq(pobj_t obj) {
		//done.
		if (IS_OBJ(obj)) {
			if (IS_SEQ(obj)) {
				return TRUE;
			}
		}
		return FALSE;
	}
	bool is_integer(pobj_t obj) {
		//done.
		if (IS_OBJ(obj)) {
			if (IS_INTEGER(obj)) {
				return TRUE;
			}
		}
		return FALSE;
	}
	bool is_short_atom(pobj_t obj) {
		//done.
		if (IS_OBJ(obj)) {
			if (IS_FLOAT(obj)) {
				return TRUE;
			}
		}
		return FALSE;
	}
	bool is_atom_ptr(pobj_t obj) {
		//done.
		if (IS_OBJ(obj)) {
			if (IS_DOUBLE_PTR(obj)) {
				return TRUE;
			}
		}
		return FALSE;
	}
	bool is_ubinary(pobj_t obj) {
		//done.
		if (IS_OBJ(obj)) {
			if (IS_UBINARY(obj)) {
				return TRUE;
			}
		}
		return FALSE;
	}

	integer get_length(pobj_t ret) {
		//done.
		if (IS_OBJ(ret)) {
			ret = get_non_dup(ret);
			if (IS_REP(ret)) {
				if (ret->rep != NULL) {
					return ret->rep->count;
				}
			}
			if (IS_SEQ(ret)) {
				if (ret->s != NULL) {
					return ret->s->length;
				}
				else {
					return 0;
				}
			}
			if (IS_UBINARY(ret)) {
				if (ret->ubin != NULL) {
					return ret->ubin->size;
				}
			}
		}
		return (-1);
	}

	pobj_t c_seq_at(pobj_t ret, integer index, bool is_force_array) { // read-only
																	  //done. needs work.
		if (IS_OBJ(ret)) {
			ret = get_non_dup(ret);
			if (IS_REP(ret)) {
				if (ret->rep != NULL) {
					index = c_get_index(index, ret->rep->count);
					if (index >= 0) {
						if (index < ret->rep->count) {
							return ret->rep->p;
						}
					}
				}
			}
			else if (IS_SEQ(ret)) {
				pelement_t ele;
				if (ret->s != NULL) {
					if (is_force_array == TRUE) {
						if (ret->s->position != -1) {
							flush_to_array(ret->s);
						}
					}
					if (ret->s->position == -1) { // looks at array of pointers when position is (-1)
						index = c_get_index(index, ret->s->length);
						if (index >= 0) {
							if (index < ret->s->length) {
								return ret->s->a[index]; // "blocks" [] are always offsets (which start at zero, 0)
							}
						}
					}
					ele = c_element_seek(ret->s, index);
					if (ele != NULL) {
						return ele->p;
					}
				}
			}
		}
		return NULL;
	}
	unsigned char * c_ubin_at(pobj_t ret, integer index) { // returns a pointer for read/write access
														   //done. needs work.
		if (IS_OBJ(ret)) {
			ret = get_non_dup(ret);
			if (IS_UBINARY(ret)) {
				if (ret->ubin != NULL) {
					index = c_get_index(index, ret->ubin->size);
					if (index >= 0) {
						if (index < ret->ubin->size) {
							return &ret->ubin->bytes[index]; // "blocks" [] are always offsets (which start at zero, 0)
						}
					}
				}
			}
		}
		return NULL;
	}

	bool c_modify_seq_range(pobj_t ret, integer index_start, integer index_stop, pobj_t orig) {
		// I need to work on this.

		//done. needs work.
		pseq_t s;
		integer len;

		/*
		In "c_" all indexes start at zero (0)
		Indexes for a 4-element sequence:

		-5-4-3-2-1
		0 1 2 3(4)
		b a

		a is index_start
		b is index_stop

		a  0 1 2 3 4
		b  4 0 1 2 3

		Inserts "orig" at index "a", without deleting elements

		a=0, prepend (NOTE: inserts before index)
		a=4, append, or use a=(-1) for append

		*/

		if (IS_OBJ(ret)) {
			len = get_length(ret);
			if (len < 0) {
				return FALSE;
			}
			index_start = c_get_index(index_start, len);
			if (index_start < 0) {
				return FALSE;
			}
			index_stop = c_get_index(index_stop, len);
			if (index_stop < -1) { // allow for zero element append (0,-1)
				return FALSE;
			}
			// compare:
			if (index_stop == len) { // we need to make "index_stop" equal negative one (-1) in this case
									 //if (index_start > 0) {
									 //      return FALSE;
									 //}
				index_stop = -1;
			}
			if (index_start > index_stop + 1) {
				return FALSE;
			}
			// create a completely new "seq"
			if (IS_DUP(ret)) {
				if (ret->dup == NULL) {
					return FALSE;
				}
				// clone for writing
				if (ret->dup->p == NULL) {
					return FALSE;
				}
				if (!IS_SEQ(ret->dup->p)) {
					pobj_t c;
					c = local_clone_obj(ret->dup->p, FALSE); // MAKE LOCAL COPY
					if (ret->dup->count > 1) {
						ret->dup->count--;
					}
					else {
						local_delete_obj(ret->dup->p);
						free(ret->dup);
					}
					memcpy(ret, c, sizeof(obj_t));
					//ret->reserved = c->reserved;
					//ret->reserved1 = c->reserved1;
					free(c);
					return c_modify_seq_range(ret, index_start, index_stop, orig);
				}
				s = c_clone_seq_modify_range(ret->dup->p->s, index_start, index_stop, orig);
				if (s == NULL) {
					return FALSE;
				}
				if (ret->dup->count > 1) {
					ret->dup->count--;
				}
				else {
					local_delete_obj(ret->dup->p);
					free(ret->dup);
				}
				ret->s = s;
				ret->flag = SEQ;
				return TRUE;
			}
			if (IS_REP(ret)) {
				if (ret->rep == NULL) {
					return FALSE;
				}
				len = ret->rep->count - index_stop + index_start - 1;
				if (len < 0) {
					return FALSE;
				}
				s = c_expand_repeat_modify_range(ret->rep, index_start, index_stop, orig);
				local_delete_obj(ret->rep->p);
				free(ret->rep);
				ret->s = s;
				ret->flag = SEQ;
				return TRUE;
			}
			if (IS_SEQ(ret)) {
				if (ret->s == NULL) {
					pobj_t ob;
					ob = get_non_dup(orig);
					if (IS_SEQ(ob) || IS_REP(ob)) {
						ob = local_new_obj(orig);
					}
					else {
						s = new_seq_one_element(orig);
						ob = allocate_obj(s, SEQ, 0);
					}
					memcpy(ret, ob, sizeof(obj_t));
					free(ob);
					return TRUE;
				}
				// delete range
				if (index_start <= index_stop) {
					c_delete_seq_range(ret->s, index_start, index_stop);
				}
				// insert elements at current position
				return c_insert_slice(ret->s, index_start, orig);
			}
		}
		return FALSE;
	}

	void print_obj(pobj_t ret) {
		//done. needs work.
		if (IS_OBJ(ret)) {
			integer i, len;
			ret = get_non_dup(ret);
			switch (ret->flag) {
			case INTEGER:
#ifdef IS_64BIT
				printf("%lli", (long long)ret->i);
#else
				printf("%i", (int)ret->i);
#endif
				break;
			case FLOAT:
				printf("%e", ret->f);
				break;
			case DOUBLE_PTR:
#ifdef IS_64BIT
				printf("%Le", *(ret->d_ptr));
#else
				printf("%e", *(ret->d_ptr));
#endif
				break;
			case UBINARY_DATA:
				printf("[");
				if (ret->ubin != NULL) {
					len = ret->ubin->size;
					if (len > 0) {
						// Go thru the sequence printing each element:
						integer num;
						num = ret->ubin->bytes[0];
#ifdef IS_64BIT
						printf("%llu", (long long)num);
#else
						printf("%u", num);
#endif
						if ((num >= 32) && (num <= 126)) {
							printf("\'%c\'", (char)num);
						}
						for (i = 1; i < len; i++) {
							num = ret->ubin->bytes[i];
#ifdef IS_64BIT
							printf(",%llu", (long long)num);
#else
							printf(",%u", num);
#endif
							if ((num >= 32) && (num <= 126)) {
								printf("\'%c\'", (char)num);
							}
						}
					}
				}
				printf("]");
				break;
			default:
				printf("{");
				len = get_length(ret);
				if (len > 0) {
					// Go thru the sequence printing each element:
					print_obj(c_seq_at(ret, 0, FALSE));
					for (i = 1; i < len; i++) {
						printf(",");
						print_obj(c_seq_at(ret, i, FALSE));
					}
				}
				printf("}");
				break;
			}
			return;
		}
		// error
	}


	// local function definitions go here:


	// Local function:
	void register_obj(pobj_t obj) {
		//done
		pobj_next_t ptr;
		ptr = (pobj_next_t)malloc(sizeof(obj_next_t));
		ptr->p = obj;
		ptr->next = all_objects;
		all_objects = ptr;
	}

	void delete_seq(pseq_t s) {
		//done.
		// written like this for debugging purposes
		integer pos, len;
		pelement_t ele, tmp;
		if (s != NULL) {
			if (s->position == -1) {
				free(s->a);
			}
			len = s->length;
			if (len > 0) {
				ele = s->first;
				if (ele != NULL) {
					for (pos = 0; pos < len; pos++) {
						local_delete_obj(ele->p);
						tmp = ele->next;
						free(ele);
						ele = tmp;
					}
				}
			}
		}
		free(s);
	}

	void local_delete_obj(pobj_t obj) {
		//done, except for error message
		// recursively delete "obj"
		if (obj == NULL) {
			return;
		}
		switch (obj->flag) {
			// it falls-thru unless there is the "break" keyword
		case DOUBLE_PTR:
			// uses fall-thru
			free(obj->d_ptr);
		case INTEGER:
			// uses fall-thru
		case FLOAT:
			// uses fall-thru
			free(obj);
			break;
		case SEQ:
			delete_seq(obj->s);
			free(obj);
			break;
		case DUP:
			if (obj->dup->count > 1) {
				obj->dup->count--;
			}
			else {
				local_delete_obj(obj->dup->p);
				free(obj->dup);
			}
			free(obj); // PS. remember to free the object!
			break;
		case REP:
			local_delete_obj(obj->rep->p);
			free(obj->rep);
			free(obj);
			break;
		case UBINARY_DATA:
			free(obj->ubin->bytes);
			free(obj->ubin);
			free(obj);
			break;
		default:
			// ???
			printf("Error in de-allocating data.");
			system("pause");
		}
	}

	pobj_t allocate_obj(void * val1, integer flags, integer extended) {
		//done.
		pobj_t obj;
		obj = (pobj_t)malloc(sizeof(obj_t));
		if (obj == NULL) {
			return NULL;
		}
		obj->reserved1 = val1; // represents data
#ifdef IS_8BIT
		obj->is_8bit = 1;
		obj->arch = extended;
#else
		obj->reserved = 0; // set integer to zero (0)
		obj->arch = MyEuphoria_arch; // represents architecture
#ifndef IS_16BIT
		obj->extended8 = POINTER_SIZE + (IS_EQUAL_DBL ? 128 : 0); // so as not to confuse pointer size OR long double size
#endif // !IS_16BIT
		obj->extended = (unsigned short)extended;
#endif // !IS_8BIT
		obj->flag = flags; // represents type
		return obj;
	}
	pdup_t allocate_dup(pobj_t p, integer count) {
		//done.
		pdup_t dup;
		dup = (pdup_t)malloc(sizeof(dup_t));
		if (dup == NULL) {
			return NULL;
		}
		dup->count = count;
		dup->p = p;
		return dup;
	}
	prep_t allocate_rep(pobj_t p, integer count) {
		//done.
		prep_t rep;
		rep = (prep_t)malloc(sizeof(rep_t));
		if (rep == NULL) {
			return NULL;
		}
		rep->count = count;
		rep->p = p;
		return rep;
	}
	pubin_t allocate_ubinary(unsigned char *ustr, integer size) {
		//done.
		pubin_t s;
		s = (pubin_t)malloc(sizeof(ubin_t));
		if (s == NULL) {
			return NULL;
		}
		s->size = size;
		s->bytes = ustr;
		return s;
	}

	inline integer c_get_index(integer index, integer len) {
		//done.
		if (len >= 0) {
			if (index < 0) {
				index++;
				index += len;
			}
			if (index >= 0) { // prepend is "zero" (0)
				if (index <= len) { // append is "len" (len)
					return index;
				}
			}
		}
		return -1; // error, negative one (-1)
	}
	inline pseq_t new_empty_seq(void) {
		//done.
		pseq_t ptr;
		ptr = (pseq_t)malloc(sizeof(seq_t));
		if (ptr == NULL) {
			return NULL;
		}
		return (pseq_t)memset(ptr, 0, sizeof(seq_t));
	}
	pseq_t new_seq_one_element(pobj_t ob) {
		//done.
		pseq_t s;
		pelement_t ele;
		ele = (pelement_t)malloc(sizeof(element_t));
		if (ele == NULL) {
			return NULL;
		}
		ele->p = local_new_obj(ob);
		ele->prev = NULL;
		ele->next = NULL;
		s = (pseq_t)malloc(sizeof(seq_t));
		if (s == NULL) {
			return NULL;
		}
		s->length = 1;
		s->position = 1;
		s->first = ele;
		s->last = ele;
		s->ptr = ele;
		return s;
	}

	inline pobj_t get_non_dup(pobj_t ret) {
		//done.
		return IS_DUP(ret) ? ret->dup->p : ret;
	}
	pseq_t expand_repeat(prep_t r, bool is_writeable) { // dont use frequently
		//done.
		pseq_t s;
		pelement_t ele, prev;
		integer c, i;
		if (r == NULL) {
			return NULL;
		}
		s = new_empty_seq();
		if (s == NULL) {
			return NULL;
		}
		c = r->count;
		if (c > 0) {
			ele = (pelement_t)malloc(sizeof(element_t));
			if (ele == NULL) {
				free(s);
				return NULL;
			}
			s->length = c;
			s->first = ele;
			if (is_writeable == TRUE) {
				ele->p = local_clone_obj(r->p, is_writeable);
			}
			else {
				ele->p = local_new_obj(r->p);
			}
			ele->prev = NULL;
			prev = ele;
			for (i = 1; i < c; i++) {
				ele = (pelement_t)malloc(sizeof(element_t));
				if (ele == NULL) {
					return NULL;
				}
				if (is_writeable == TRUE) {
					ele->p = local_clone_obj(r->p, is_writeable);
				}
				else {
					ele->p = local_new_obj(r->p);
				}
				ele->prev = prev;
				prev->next = ele;
				prev = ele;
			}
			ele->next = NULL;
			s->last = ele;
		}
		return s;
	}

	bool flush_to_array(pseq_t s) {
		//done.
		pobj_t *a; // an array of "pobj_t"
		pelement_t ele;
		integer len;
		if (s->position != -1) {
			len = s->length;
			if (len == 0) {
				a = (pobj_t *)malloc(len * sizeof(pobj_t));
				if (a == NULL) {
					return FALSE;
				}
				s->a = a;
				s->position = -1; // indicates that there is an array
				ele = s->first;
				while (len > 0) {
					*a = ele->p; // "C" pointer arithmetic
					a++; // separate line for debugging
					ele = ele->next;
					len--;
				}
			}
		}
		return TRUE;
	}
	pseq_t concat_seq(pseq_t * from, integer num) { // takes an array of "pseq_t" pointers, of length "num"
		//done.
		// idea: concat_obj()
		integer i, j, len;
		pelement_t prev, ele1, ele_from;
		pseq_t s, tmp;
		s = new_empty_seq();
		if (s == NULL) {
			return NULL;
		}
		ele1 = NULL;
		prev = NULL;
		for (i = 0; i < num; i++) {
			tmp = from[i];
			len = tmp->length;
			if (len > 0) {
				prev = (pelement_t)malloc(sizeof(element_t));
				if (prev == NULL) {
					return NULL;
				}
				if (s->first == NULL) {
					s->first = prev;
				}
				else {
					ele1->next = prev;
				}
				prev->prev = ele1; // NULL or prev
				// sequence s1 to s2:
				ele_from = tmp->first;
				prev->p = local_new_obj(ele_from->p);
				s->length += len;
				for (j = 2; j <= len; j++) {
					ele1 = (pelement_t)malloc(sizeof(element_t));
					if (ele1 == NULL) {
						return NULL;
					}
					prev->next = ele1; // prev is previous node of the "doubly-linked" list
					ele1->prev = prev;
					ele_from = ele_from->next;
					ele1->p = local_new_obj(ele_from->p);
					prev = ele1;
				}
				prev->next = ele1;
				ele1 = prev; // required for "loop-back"
			}
		}
		if (prev != NULL) {
			prev->next = NULL;
			s->last = ele1;
		}
		return s;
	}


	// NOTE: "c" functions use offset, like in "C/CPP", which starts at 0. Others use index which starts at 1. Choose one or the other.

	// element access:

	//#ifdef USE_C_OFFSETS
	pelement_t c_element_seek(pseq_t myseq, integer index) {
		//done. needs work. bookmark
		integer t1, t2;
		pelement_t tmp;
		// In "C", offsets start at zero (0)
		//index = index + 1; // subtract one from the opposite side
		if (myseq == NULL) {
			return NULL;
		}
		index = c_get_index(index, myseq->length);
		if (index < 0) { // error
			return NULL;
		}
		if (index == myseq->length) { // append
									  // these statements are required for append to work:
			myseq->position = 0; // position starts at one (1), append is zero (0)
			myseq->ptr = NULL;
			return NULL;
		}
		if (myseq->position == -1) { // if position equals negative one (-1), then "a" is an array of pointers
			free(myseq->a); // dynamically free "a" because "seek" uses "ptr"
			myseq->a = NULL;
			myseq->position = 0;
			//myseq->ptr = NULL; // "a = NULL" sets "ptr" because share the same memory location, this is called a "union"
			return c_element_seek(myseq, index);
		}
		if (index == myseq->position - 1) {
			// myseq->position = index + 1; // they are already equal in this case
			return myseq->ptr;
		}
		t2 = myseq->position - index - 1;
		//if (t2 == 0) {
		//      return;
		//}
		if (t2 < 0) {
			t2 = (-t2);
		}
		t1 = index;
		if (t1 < t2) {
			// first, (cursor is going down)
			if (myseq->first == NULL) {
				myseq->first = myseq->last;
			}
			myseq->ptr = myseq->first;
			myseq->position = 1;

			if (index == 0) {
				return myseq->ptr;
			}

			//while (t1 > 0) {
			//      myseq->ptr = myseq->ptr->next; //here
			//      (t1--); // decrement
			//}
			//return myseq->ptr;
		}
		t1 = myseq->length - index - 1;
		if (t1 < t2) {
			// last, (cursor is going up)
			if (myseq->last == NULL) {
				myseq->last = myseq->first;
			}
			myseq->ptr = myseq->last;
			myseq->position = myseq->length;

			if (index == myseq->length - 1) {
				return myseq->ptr;
			}

			//while (t1 > 0) {
			//      myseq->ptr = myseq->ptr->prev; //here
			//      (t1--); // decrement
			//}
			//return myseq->ptr;
		}
		tmp = myseq->ptr;
		t1 = myseq->position - index - 1; // using adjusted "myseq->position"
										  // cursor is going up (left)
		while (t1 > 0) {
			if (tmp == NULL) {
				return NULL;
			}
			tmp = tmp->prev; // NOTE: prev
			(t1--); // decrement
		}
		if (t1 < 0) {
			t1 = (-t1);
		}
		// cursor is going down (right)
		while (t1 > 0) {
			if (tmp == NULL) {
				return NULL;
			}
			tmp = tmp->next; // NOTE: next
			(t1--); // decrement
		}
		myseq->position = index + 1;
		myseq->ptr = tmp;
		return tmp;
	}

	bool c_delete_seq_range(pseq_t s, integer index_start, integer index_stop) {
		//done. needs work.
		integer i;
		pelement_t ele, tmp, prev;
		if (index_start <= index_stop) {
			if (s != NULL) {
				ele = c_element_seek(s, index_start);
				if (ele != NULL) {
					prev = ele->prev;
					for (i = index_start; i <= index_stop; i++) {
						local_delete_obj(ele->p);
						tmp = ele->next;
						free(ele);
						ele = tmp;
					}
					// "prev" is now "last"
					if (prev != NULL) {
						prev->next = ele;
					}
					if (ele != NULL) {
						ele->prev = prev;
					}
					if (index_start == 0) {
						s->first = ele;
					}
					s->length--;
					if (index_stop == s->length) {
						s->last = prev;
					}
					s->length -= (index_stop - index_start);
					// these values may need to be corrected:
					s->ptr = prev;
					s->position = index_start;
					return TRUE;
				}
			}
		}
		return FALSE;
	}

	// write-only
	bool c_insert_slice(pseq_t s, integer index, pobj_t orig) {
		//done. needs work.
		pobj_t r, ret;
		pseq_t tmp;
		pelement_t ele;
		if (orig == NULL) {
			return FALSE;
		}
		if (s == NULL) {
			return FALSE;
		}
		index = c_get_index(index, s->length);
		if (index < 0) {
			return FALSE;
		}
		if (index > s->length) {
			return FALSE;
		}
		ret = get_non_dup(orig);
		if (!IS_SEQ(ret)) {
			if (IS_REP(ret)) {
				if (ret->rep == NULL) {
					return FALSE;
				}
				else {
					tmp = expand_repeat(ret->rep, FALSE);
				}
			}
			else {
				tmp = new_seq_one_element(orig); // it should work this way, without a memory leak
			}
			r = allocate_obj(tmp, SEQ, 0);
		}
		else {
			r = local_clone_obj(orig, FALSE); // make duplicates, when necessary
		}
		if (r->s->length > 0) { // handle empty sequence
			if (s->length == 0) {
				s->first = r->s->first;
				s->last = r->s->last;
			}
			else if (index == s->length) {
				// append to end of sequence
				r->s->first->prev = s->last;
				s->last->next = r->s->first;
				s->last = r->s->last;
				// appending, "position" and "ptr" would be the same in this case.
			}
			else if (index == 0) {
				// prepend to the beginning of sequence
				r->s->last->next = s->first;
				s->first->prev = r->s->last;
				s->first = r->s->first;
				if (s->position > 0) { // if position is initialized then
					s->position += r->s->length;
				}
			}
			else {
				// inserting into the middle of sequence
				ele = c_element_seek(s, index);
				if (ele == NULL) {
					// free temporary variables:
					free(r->s);
					free(r);
					return FALSE;
				}
				s->ptr->prev->next = r->s->first;
				r->s->first->prev = s->ptr->prev;
				s->ptr->prev = r->s->last;
				r->s->last->next = s->ptr;
			}
			s->length += r->s->length;
		}
		// free temporary variables:
		free(r->s);
		free(r);
		return TRUE;
	}
	pseq_t c_new_seq(pseq_t s_from, integer index, integer range_len) {
		//done. needs work.
		pseq_t s;
		pelement_t ele, ele1, ele_from;
		integer i;
		s = new_empty_seq();
		if (range_len > 0) {
			s->length = range_len;
			ele = (pelement_t)malloc(sizeof(element_t));
			if (ele == NULL) {
				return NULL; // error
			}
			ele->prev = NULL;
			s->first = ele;
			ele_from = c_element_seek(s_from, index);
			ele->p = local_new_obj(ele_from->p);
			for (i = 2; i <= range_len; i++) {
				ele1 = (pelement_t)malloc(sizeof(element_t));
				if (ele1 == NULL) {
					return NULL; // error
				}
				ele1->prev = ele;
				ele->next = ele1; // prev is previous node of the "doubly-linked" list
				ele_from = ele_from->next;
				ele1->p = local_new_obj(ele_from->p);
				ele = ele1;
			}
			ele->next = NULL;
			s->last = ele;
		}
		return s;
	}
	pseq_t c_clone_seq_modify_range(const pseq_t s, integer index_start, integer index_stop, pobj_t orig) {
		//done. needs work.
		pseq_t ret;
		pseq_t from[3];
		bool flag;
		//index = index + 1; // subtract one from the opposite side
		//index_start = index_start + 1;
		//index_stop = index_stop + 1;
		orig = get_non_dup(orig);
		if (IS_SEQ(orig)) {
			flag = FALSE;
			from[1] = orig->s;
		}
		else if (IS_REP(orig)) {
			flag = TRUE;
			from[1] = expand_repeat(orig->rep, FALSE);
		}
		else {
			flag = TRUE;
			from[1] = new_seq_one_element(orig);
		}
		from[0] = c_new_seq(s, 0, index_start);
		from[2] = c_new_seq(s, index_stop + 1, s->length - index_stop - 1);
		ret = concat_seq(from, 3); // could make this faster by handling REP within a pobj_t "concat_seq()"
		delete_seq(from[0]);
		delete_seq(from[2]);
		if (flag == TRUE) {
			delete_seq(from[1]); // delete created sequence
		}
		return ret;
	}
	pseq_t c_expand_repeat_modify_range(prep_t rep, integer index_start, integer index_stop, pobj_t orig) {
		//done
		pseq_t s;
		integer len;
		len = rep->count + index_start - index_stop - 1;
		if (len < 0) {
			s = NULL;
		}
		else {
			if (len > 0) {
				rep->count = len;
				s = expand_repeat(rep, FALSE);
			}
			else {
				s = new_empty_seq();
			}
			if (c_insert_slice(s, index_start, orig) == FALSE) {
				delete_seq(s);
				s = NULL;
			}
		}
		return s;
	}


#ifdef __cplusplus
} // extern "C"
#endif
