/**
 * touchdb.h
 *
 * @author chosen0ne
 * @date 2012-06-10
 *
 * Copyright (C) by chosen0ne
 */

#ifndef TOUCHDB_H
#define TOUCHDB_H

#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>

#include "touchdb_common.h"
#include "touchdb_pool.h"
#include "touchdb_map.h"
#include "touchdb_chain.h"
#include "touchdb_rbtree.h"
#include "touchdb_str.h"
#include "touchdb_cow_map.h"
#include "touchdb_btree.h"
#include "touchdb_util.h"
#include "touchdb_array.h"

#define TOUCHDB_VERSION					"00_01_00"
#define TOUCHDB_INFO_FILE				"touchdb.info"
#define TOUCHDB_DATA_FILE_PREFIX		"touchdb"
#define TOUCHDB_DATA_FILE_SUFFIX		".dat"
#define TOUCHDB_INFO_FILE_SIZE			40 * 1024 * 1024

#define set_addr(addr, p, idx)				\
			(addr)->pool_idx = idx;			\
			(addr)->offset = p


typedef union {
	touchdb_map_t 		map;
	touchdb_rbtree_t	rbtree;
	touchdb_chain_t		chain;
} touchdb_idx_table;

typedef struct {
	/* version info of touchdb, the content is 'TOUCHDBxx_xx_xx' */
	char 				magic_num[16];
	int					mode;
	touchdb_idx_table	idx_table;
	int					lock_granularity;
	char				db_path[1024];

	/* shared memory pool list, the default size of the pool is 100MB */
	touchdb_pool_t		pool_list[TOUCHDB_POOL_LIST_SIZE];
} touchdb;

typedef enum {
	TOUCHDB_MAP,
	TOUCHDB_CHAIN,
	TOUCHDB_COW_MAP,
	TOUCHDB_RBTREE,
	TOUCHDB_BTREE
} touchdb_mode;


void touchdb_default_logger(int level, const char *fmt, ...);
touchdb* touchdb_new(const char *dat_path, int lock_granularity, touchdb_mode mode, touchdb_log_t log_handler);
touchdb* touchdb_new2(const char *config, touchdb_log_t log_handler);

void* touchdb_malloc(touchdb_addr_t *addr, int size);
void touchdb_free(touchdb_addr_t *addr);

void* touchdb_ptr_of(touchdb_addr_t *addr);
void touchdb_addr_of(touchdb_addr_t *addr, void *p);

const char* touchdb_put(touchdb *db, const char *k, const char *v);
const char* touchdb_get(touchdb *db, const char *);
int	touchdb_size(touchdb *db);
void touchdb_iter(touchdb *db, touchdb_map_iterator_t iter);

const touchdb_val_t* touchdb_put_str(touchdb *db, const char *k, const char *v);
const char* touchdb_get_str(touchdb *db, const char *k);

void touchdb_list_push(touchdb *db, const char *k, touchdb_val_t *v);
const touchdb_val_t* touchdb_list_pop(touchdb *db, const char *k);
const touchdb_val_t* touchdb_list_node_idx(touchdb *db, const char *k, int idx);
touchdb_array_t* touchdb_list_range(touchdb *db, const char *k, int s, int e);

int touchdb_list_size(touchdb *db, const char *k);


#endif
