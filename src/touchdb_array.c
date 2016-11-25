/**
 * touchdb_array.c
 *
 * @author chosen0ne
 * @date 2012-06-12
 *
 * Copyright (C) by chosen0ne
 */

#include "touchdb_array.h"

touchdb_array_t*
touchdb_array_new(touchdb_addr_t *addr, int n, int elt_sz){
	touchdb_array_t		*array;
	touchdb_val_t		*elts;

	if( (array = touchdb_malloc(addr, sizeof(touchdb_array_t))) == NULL ){
		touchdb_log(TOUCHDB_LOG_ERR, "[touchdb_array_new]Alloc mem for array error!");
		exit(1);
	}

	if( (elts = touchdb_malloc(&array->elts, n * elt_sz)) == NULL ){
		touchdb_log(TOUCHDB_LOG_ERR, "[touchdb_array_new]Alloc mem for array->elts error!");
		exit(1);
	}

	array->nelts = 0;
	array->nalloc = n;
	array->elt_sz = elt_sz;

	return NULL;
}

touchdb_val_t*
touchdb_array_push(touchdb_array_t *array){
	touchdb_val_t		*val, *elts;
	touchdb_addr_t		addr;
	int					i;

	// array is full! Allocate 2 times space.
	if(array->nelts == array->nalloc){

		val = touchdb_malloc(&addr, array->nalloc * 2, array->elt_sz);

		for(elts = touchdb_ptr_of(&array->elts), i = 0; i < array->nelts; i++){
			*(val + i) = *(elts + i);
		}

		touchdb_addr_of(array->elts, val);
	}

	val = touchdb_ptr_of(&array->elts);
	array->nelts++;

	return (val + array->nelts);
}



