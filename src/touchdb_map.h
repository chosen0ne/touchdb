/**
 * touchdb_map.h
 *
 * @author chosen0ne
 * @date 2012-06-15
 *
 * Copyright (C) by chosen0ne
 */

#ifndef TOUCHDB_MAP_H
#define TOUCHDB_MAP_H

#include "touchdb_common.h"
#include <stdbool.h>

typedef struct {
	touchdb_addr_t 		prev;				/* touchdb_map_entry */
	touchdb_addr_t	 	next;				/* touchdb_map_entry */
	int					hash;
	touchdb_addr_t		key;				/* char */
	touchdb_val_t		val;
} touchdb_map_entry_t;

typedef struct {
	touchdb_addr_t		header;				/* touchdb_map_entry */
	touchdb_addr_t		tail;				/* touchdb_map_entry */
	int					size;
} touchdb_map_bulk_t;

typedef struct {
	touchdb_addr_t		bulk_list;			/* touchdb_map_bulk* */
	int					bulk_list_count;
	int					size;
	int 				capacity;
	float				load_factor;
} touchdb_map_t;

typedef void (*touchdb_map_iterator_t)(const char *k, const char *v);
typedef bool (*touchdb_map_deletable_t)(const char *k, void *v);

touchdb_map_t* touchdb_map_init(touchdb_map_t *map, int capacity, int ld_factor);
const char* touchdb_map_put(touchdb_map_t *map, const char *k, const char *v);
const char* touchdb_map_get(touchdb_map_t *map, const char *k);
const char* touchdb_map_del(touchdb_map_t *map, const char *k);

const touchdb_val_t* touchdb_map_put2(touchdb_map_t *map, const char *k,
		const touchdb_val_t *v);
const touchdb_val_t* touchdb_map_get2(touchdb_map_t *map, const char *k);
//touchdb_chain_t* touchdb_map_del(touchdb_map_t *map, touch_db_map_deletable_t *deletable);

bool touchdb_map_contains(touchdb_map_t *map, const char *k);
int	touchdb_map_size();
void touchdb_map_iter(touchdb_map_t *map, touchdb_map_iterator_t iter);
void touchdb_map_bulk_info();
int touchdb_map_cmp(touchdb_map_t *m1, touchdb_map_t *m2);
void touchdb_map_destory(touchdb_map_t *m);

#endif
