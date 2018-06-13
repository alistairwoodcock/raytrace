#ifndef ARRAY
#define ARRAY
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

/*
	Data Structure for these arrays

	[u64 |u64  |u64         |...data...]
	[size|count|element_size_bytes|...data...]

*/

#define STARTING_OFFSET 3
#define get_full_array(array) ((void*)(((u64*)(*array)) - STARTING_OFFSET))
#define get_counter_pointer(array) ((u64*)get_full_array(array))+1
#define get_array_size(array) (((u64*)get_full_array(array))[0])
#define get_array_count(array) (((u64*)get_full_array(array))[1])
#define get_element_size(array) (((u64*)get_full_array(array))[2])

const u64 STARTING_BYTES = sizeof(u64)*STARTING_OFFSET; // u64 + u64


#define a_ref(array) ((void**) &(array)) //just used to get the pointer to the array pointer
#define a_new(type, size) (type *)(create_new_array(sizeof(type), size))
#define a_size(array) get_array_size(a_ref(array))
#define a_count(array) get_array_count(a_ref(array))
#define a_push(array,val) (array_maybe_grow(a_ref(array),1) , (array)[(*(get_counter_pointer(a_ref(array))))++] = (val))
#define a_grow(array, amount) array_grow(&(array), amount)
#define a_print(array) array_print(a_ref(array))

void array_print(void** array) {
	u64* full = get_full_array(array);
	u64 size = full[0];
	u64 count = full[1];
	u64 el_size = full[2];

	printf("array[%llu|%llu|%llu|...\n", size, count, el_size);
}


void* create_new_array(u64 el_size_bytes, u64 size)
{
	u64* full_array = (u64*)malloc(STARTING_BYTES + 
									(size * el_size_bytes));

	void* exposed_array = (void*)(full_array+STARTING_OFFSET);

	full_array[0] = size;
	full_array[1] = 0;
	full_array[2] = el_size_bytes;

	return exposed_array;
}

void array_grow(void** array, u64 increase){
	u64* full = null;
	u64 size = 1;
	u64 count = 0;
	u64 el_size = sizeof(u64);

	if(*array != null){
		full = (u64*)get_full_array(array);
		size = full[0];
		count = full[1];
		el_size = full[2];	
	}
	
	u64 new_size = (size + increase);
	u64* new_arr = (u64*)realloc((void*)full, STARTING_BYTES + 
											   (new_size * el_size));
	
	if(new_arr)
	{
		new_arr[0] = new_size;
		new_arr[1] = count;
		new_arr[2] = el_size;

		*array = (void*)(new_arr + STARTING_OFFSET);
	}
}

void array_maybe_grow(void** array, u64 increase)
{
	if(*array == null){
		array_grow(array, 1);
		return;	
	}

	u64 size = get_array_size(array);
	u64 count = get_array_count(array);
	
	if(count + increase >= size){
		printf("we are growing the array\n");
		array_grow(array, 2*size);
	}
}


#endif