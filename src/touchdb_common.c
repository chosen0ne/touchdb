/**
 * touchdb_common.c
 *
 * @author chosen0ne
 * @date 2012-06-11
 *
 * Copyright (C) by chosen0ne
 */

#include "touchdb_common.h"
#include "touchdb_str.h"
#include "touchdb_map.h"
#include "touchdb_chain.h"

touchdb_addr_t	NULL_ADDR = {-1, NIL};

extern int touchdb_str_cmp(touchdb_val_t *s1, touchdb_val_t *s2);
extern int touchdb_chain_cmp(touchdb_val_t *c1, touchdb_val_t *c2);
extern int touchdb_map_cmp(touchdb_map_t *m1, touchdb_map_t *m2);

extern void touchdb_str_destory(touchdb_val_t *s);
extern void touchdb_chain_destory(touchdb_chain_t *c);
extern void touchdb_map_destory(touchdb_map_t *m);

int
touchdb_val_cmp(touchdb_val_t *a, touchdb_val_t *b){
	if(a->type != b->type)
		return -1;
	switch(a->type){
		//case TOUCHDB_VAL_INT:
		//	return touchdb_int_cmp(a, b);
//		case TOUCHDB_VAL_STR:
//			return touchdb_str_cmp(a, b);
//		case TOUCHDB_VAL_CHAIN:
//			return touchdb_chain_cmp(a, b);
		case TOUCHDB_VAL_MAP:
			return touchdb_map_cmp((touchdb_map_t *)a, (touchdb_map_t *)b);
	}
}

void
touchdb_val_destroy(touchdb_val_t *val){
	switch(val->type){
		//case TOUCHDB_VAL_INT:
		//	return touchdb_int_destroy((touchdb_int_t *)val);
		case TOUCHDB_VAL_STR:
			touchdb_str_destory(val);
			break;
//		case TOUCHDB_VAL_CHAIN:
//			touchdb_chain_destory((touchdb_chain_t *)val);
		case TOUCHDB_VAL_MAP:
			touchdb_map_destory((touchdb_map_t *)val);
			break;
	}
}
