/**
 * touchdb_common.h
 *
 * @author chosen0ne
 * @date 2012-06-11
 *
 * Copyright (C) by chosen0ne
 */

#ifndef TOUCHDB_COMMON_H
#define TOUCHDB_COMMON_H
#include <stdbool.h>

#define NIL 0

#define TOUCHDB_POOL_LIST_SIZE 			10
#define TOUCHDB_DEFAULT_POOL_SIZE 		100 * 1024 * 1024

typedef unsigned int ptr_offset;

typedef struct {
	/* index of the pool in pool list */
	int				pool_idx;

	/* offset of the pointer in the pool */
	ptr_offset		offset;
} touchdb_addr_t;

typedef enum {
	TOUCHDB_VAL_INT,
	TOUCHDB_VAL_STR,
	TOUCHDB_VAL_CHAIN,
	TOUCHDB_VAL_MAP,
	TOUCHDB_VAL_SET
} touchdb_val_type;

typedef struct {
	touchdb_val_type 	type;

	/* point to real value, touchdb_str_t, touchdb_chain_t, touchdb_map_t... */
	touchdb_addr_t		value;
} touchdb_val_t;

typedef void (*touchdb_log_t)(int level, const char *fmt, ...);

typedef enum {
	TOUCHDB_LOG_DEBUG,
	TOUCHDB_LOG_INFO,
	TOUCHDB_LOG_WARN,
	TOUCHDB_LOG_ERR
} touchdb_log_level;

extern touchdb_addr_t 	NULL_ADDR;
extern touchdb_log_t	touchdb_log;

typedef touchdb_val_t* (*put_func)(const char *k, const char *v);
typedef touchdb_val_t* (*get_func)(const char *k);

typedef touchdb_val_t* (*list_lpush_func)(const char *k, const char *v, ...);
typedef touchdb_val_t* (*list_lpull_func)(const char *k);
typedef touchdb_val_t* (*list_lrange_func)(const char *k, int offset, int len);
typedef touchdb_val_t* (*list_rpush_func)(const char *k, const char *v, ...);
typedef touchdb_val_t* (*list_rpull_func)(const char *k);
typedef touchdb_val_t* (*list_set_func)(const char *k, int idx, const char *v);
typedef touchdb_val_t* (*list_size_func)(const char *k);

typedef touchdb_val_t* (*hash_put_func)(const char *k, const char *f, const char *v, ...);
typedef touchdb_val_t* (*hash_get_func)(const char *k, const char *f);
typedef touchdb_val_t* (*hash_exists_func)(const char *k, const char *f);
typedef touchdb_val_t* (*hash_getall_func)(const char *k);
typedef touchdb_val_t* (*hash_size_func)(const char *k);

void* touchdb_malloc(touchdb_addr_t *addr, int size);
void touchdb_free(touchdb_addr_t *addr);

void* touchdb_ptr_of(touchdb_addr_t *addr);

int touchdb_val_cmp(touchdb_val_t *a, touchdb_val_t *b);
void touchdb_val_destroy(touchdb_val_t *val);

#endif
