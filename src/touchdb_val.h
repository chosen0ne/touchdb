/**
 * touchdb_val.h
 *
 * @author chosen0ne
 * @date 2012-06-14
 *
 * Copyright (C) by chosen0ne
 */

#ifndef TOUCHDB_VAL_H
#define TOUCHDB_VAL_H

#include "touchdb_common.h"

typedef enum {
	TOUCHDB_VAL_INT,
	TOUCHDB_VAL_STR,
	TOUCHDB_VAL_LIST,
	TOUCHDB_VAL_MAP
} touchdb_val_type;

typedef struct {
	touchdb_val_type 	type;
	touchdb_addr_t		value;
} touchdb_val_t;

#endif
