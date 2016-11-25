/**
 * touchdb_array.h
 *
 * @author chosen0ne
 * @date 2012-06-12
 *
 * Copyright (C) by chosen0ne
 */

#ifndef TOUCHDB_ARRAY_H_
#define TOUCHDB_ARRAY_H_

#include "touchdb_common.h"

typedef struct {
	touchdb_addr_t	elts;
	int				nelts;
	int				elt_sz;
	int				nalloc;
} touchdb_array_t;

touchdb_array_t* touchdb_array_new(touchdb_addr_t *addr, int n, int elt_sz);
touchdb_val_t* touchdb_array_push(touchdb_array_t *arr, touchdb_val_t *val);

#endif /* TOUCHDB_ARRAY_H_ */
