/**
 * touchdb_pool.h
 *
 * @author chosen0ne
 * @date 2012-06-12
 *
 * Copyright (C) by chosen0ne
 */



#ifndef TOUCHDB_POOL_H
#define TOUCHDB_POOL_H
#include <stdbool.h>
#include <stdlib.h>
#include "touchdb_common.h"
#include "touchdb_util.h"

/* 8bytes * 128 * 1024 = 1MB */
#define TOUCHDB_POOL_FREE_LIST_COUNT 128*1024

/* number of free block list that share a lock */
#define TOUCHDB_DEFAULT_LOCK_GRANULARITY 16

#define clear_addr(addr) 					\
			(&addr)->pool_idx = -1; 		\
			(&addr)->offset = NIL

#define copy_addr(dist, src)			memcpy(&dist, &src, sizeof(touchdb_addr_t))


typedef struct {
	ptr_offset 	next_offset;
	ptr_offset 	prev_offset;
	int			idx;
	int			pool_idx;
} touchdb_free_blk_hdr_t;

typedef struct {
	ptr_offset	header_offset;
	ptr_offset	tail_offset;
	int 		size;
	int 		idx;
} touchdb_free_blk_list_t;

typedef struct {
	void 	*start;
	void	*end;
} touchdb_shm_info_t;

typedef struct {
	/* array of the free block list, the size of free block is 8 bytes, 16bytes ... */
	touchdb_free_blk_list_t 	free_blk_list[TOUCHDB_POOL_FREE_LIST_COUNT];

	/* length of the free block list array */
	int							free_blk_list_count;

	ptr_offset					cur_offset;

	/* size of pool */
	int							size;

	/* the file used to 'mmap' */
	char						shm_file[16];

	/* pool index of pool list. used to fetch shared memory info */
	int							idx;
} touchdb_pool_t;

typedef struct {

} touchdb_pool_info_t;

void touchdb_pool_init(touchdb_pool_t *pool, void *ptr, int lock_granularity, bool is_inited);

void* touchdb_pool_malloc(touchdb_pool_t *pool, unsigned int size);

void touchdb_pool_free(touchdb_pool_t *pool, void *p);

ptr_offset ptr_offset_of(touchdb_pool_t *pool, void *ptr);

void* ptr_of(touchdb_pool_t *pool, ptr_offset offset);

void touchdb_pool_free_blk_info(touchdb_pool_t *pool);

void touchdb_pool_info();

#endif
