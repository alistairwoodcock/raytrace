#ifndef _ARRAY
#define _ARRAY

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

/*
    Data Structure for these arrays

    [u64 |u64  |u64         |...data...]
    [size|count|element_size_bytes|...data...]
*/

#define STARTING_OFFSET 3

//just used to get the pointer to the array pointer
#define a_ref(array) ((void**) &(array)) 

#define a_get_full_array(array) ((void*)(((u64*)(*array)) - STARTING_OFFSET))

#define a_get_counter_pointer(array) ((u64*)a_get_full_array(array))+1

#define a_get_array_size(array) (((u64*)a_get_full_array(array))[0])

#define a_get_array_count(array) (((u64*)a_get_full_array(array))[1])

#define a_get_element_size(array) (((u64*)a_get_full_array(array))[2])

const u64 STARTING_BYTES = sizeof(u64)*STARTING_OFFSET; // u64 + u64

#define a_new(type, size) (type *)(a_create_new_array(sizeof(type), size))

// returns total size of array (filled + unfilled) 
// but not including extra memory spots for meta data
#define a_size(array) a_get_array_size(a_ref(array))

// returns the number of filled spots
#define a_count(array) a_get_array_count(a_ref(array))

// grow array if too small, then allocate given val. Returns false/true on alloc fail/success
#define a_push(array,val) (a_maybe_grow(a_ref(array),1) && ((array)[(*(a_get_counter_pointer(a_ref(array))))++] = (val)))

// grow array. Returns false/true on alloc fail/success
#define a_grow(array, amount) a_array_grow(a_ref(array), amount)

#define a_print(array) array_print(a_ref(array))

#define a_free(array) a_free_allocation(a_ref(array))

void array_print(void** array) {
    u64* full = a_get_full_array(array);
    u64 size = full[0];
    u64 count = full[1];
    u64 el_size = full[2];

    printf("array[%llu|%llu|%llu|...\n", size, count, el_size);
}


void* a_create_new_array(u64 el_size_bytes, u64 size)
{
    u64* full_array = (u64*)calloc(STARTING_BYTES + (size * el_size_bytes), 1);

    void* exposed_array = (void*)(full_array+STARTING_OFFSET);

    full_array[0] = size;
    full_array[1] = 0;
    full_array[2] = el_size_bytes;

    return exposed_array;
}

bool a_array_grow(void** array, u64 increase){
    u64* full = null;
    u64 size = 1;
    u64 count = 0;
    u64 el_size = sizeof(u64);

    if(*array != null){
        full = (u64*)a_get_full_array(array);
        size = full[0];
        count = full[1];
        el_size = full[2];  
    }
    
    u64 new_size = (size + increase);
    u64* new_arr = (u64*)realloc((void*)full, STARTING_BYTES + 
                                               (new_size * el_size));
    
    if(!new_arr) return false;

    new_arr[0] = new_size;
    new_arr[1] = count;
    new_arr[2] = el_size;

    *array = (void*)(new_arr + STARTING_OFFSET);
    
    return true;
}

bool a_maybe_grow(void** array, u64 increase)
{
    if(*array == null){
        return a_array_grow(array, 1); 
    }

    u64 size = a_get_array_size(array);
    u64 count = a_get_array_count(array);
    
    if(count + increase >= size){
        return a_array_grow(array, 2*size);
    }

    return true;
}

void a_free_allocation(void** array){

    void* full = a_get_full_array(array);

    free(full);

    *array = null;
}

#endif