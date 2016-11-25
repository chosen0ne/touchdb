/* Copyright (C) by chosen0ne */

#ifndef TOUCHDB_MAP_H
#define TOUCHDB_MAP_H

#include "touchdb_common.h"
#include <stdbool.h>

typedef struct {
	int					hash;
	ptr_offset			key_offset;
	touchdb_val_t		val;
} touchdb_map_entry_t;

typedef struct {
	ptr_offset		bulk_list_offset;	/* touchdb_chain* */
	int				bulk_list_count;
	int				size;
} touchdb_map_t;

typedef void (*touchdb_map_iterator_t)(const char *k, const char *v);
typedef bool (*touchdb_map_deletable_t)(const char *k, const char *v);

touchdb_map_t* touchdb_map_init(touchdb_map_t *map, int capacity);
const char* touchdb_map_put(touchdb_map_t *map, const char *k, const char *v);
const char* touchdb_map_get(touchdb_map_t *map, const char *k);
const char* touchdb_map_del(touchdb_map_t *map, const char *k);
//touchdb_chain_t* touchdb_map_del(touchdb_map_t *map, touch_db_map_deletable_t *deletable);

bool touchdb_map_contains(touchdb_map_t *map, const char *k);
int	touchdb_map_size();
void touchdb_map_iter(touchdb_map_t *map, touchdb_map_iterator_t iter);
void touchdb_map_bulk_info();

#endif
