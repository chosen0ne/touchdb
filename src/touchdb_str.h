/**
 * touchdb_str.h
 *
 * @author chosen0ne
 * @date 2012-06-13
 *
 * Copyright (C) by chosen0ne
 */

#ifndef TOUCHDB_STR_T
#define TOUCHDB_STR_T

#include "touchdb_common.h"


typedef struct {
	unsigned int 	len;
	touchdb_addr_t	data;
} touchdb_str_t;

extern touchdb_str_t	EMPTY_STR;

int touchdb_str_cmp(touchdb_val_t *s1, touchdb_val_t *s2);

touchdb_val_t* touchdb_str_new(touchdb_addr_t *addr, const char *s);
touchdb_str_t* touchdb_str_new2(touchdb_val_t *val, const char *s);

const char* touchdb_str_data(touchdb_val_t *s);

int touchdb_str_cpy(touchdb_val_t *dest, touchdb_val_t *src, int n);

void touchdb_str_destory(touchdb_val_t *s);

#endif
