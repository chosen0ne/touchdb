/**
 * touchdb_pool.c
 *
 * @author chosen0ne
 * @date 2012-06-12
 *
 * Copyright (C) by chosen0ne
 */

#include "touchdb_pool.h"
#include <string.h>
#include <errno.h>

#define cur_ptr(pool)						(void *)((char *)shm_info[pool->idx].start + pool->cur_offset)
#define set_cur_ptr_offset(pool, offset)	pool->cur_offset += offset
#define blk_data_by_hdr(hdr)				(void *)((char *)hdr + BLK_HDR_SIZE)
#define blk_hdr_by_data(data)				(touchdb_free_blk_hdr_t *)((char *)data - BLK_HDR_SIZE)
#define free_list_header(pool, list)		(touchdb_free_blk_hdr_t *)ptr_of(pool, list->header_offset)
#define free_list_tail(pool, list)			(touchdb_free_blk_hdr_t *)ptr_of(pool, list->tail_offset)
#define blk_hdr_next(pool, hdr)				(touchdb_free_blk_hdr_t *)ptr_of(pool, hdr->next_offset)
#define blk_hdr_prev(pool, hdr)				(touchdb_free_blk_hdr_t *)ptr_of(pool, hdr->prev_offset)


extern touchdb_shm_info_t		shm_info[TOUCHDB_POOL_LIST_SIZE];

static int BLK_HDR_SIZE = sizeof(touchdb_free_blk_hdr_t);

static int
free_list_idx(int size){
	if(size % 8 == 0)
		return (size >> 3) - 1;
	return size >> 3;
}

void
touchdb_pool_init(touchdb_pool_t *pool, void *ptr, int lock_granularity, bool is_inited){
	int 	i;

	shm_info[pool->idx].start = ptr;
	shm_info[pool->idx].end = (char *)ptr + TOUCHDB_DEFAULT_POOL_SIZE;

	if(is_inited){
		touchdb_log(TOUCHDB_LOG_INFO, "[touchdb_pool_init]Load pool structure from data file.");
	}else{
		touchdb_log(TOUCHDB_LOG_INFO, "[touchdb_pool_init]Init pool structure.");

		// init free block list
		pool->free_blk_list_count = TOUCHDB_POOL_FREE_LIST_COUNT;
		for(i=0; i<pool->free_blk_list_count; i++){
			memset(pool->free_blk_list+i, 0, sizeof(touchdb_free_blk_list_t));
			(pool->free_blk_list+i)->idx = i;
		}

		pool->size = TOUCHDB_DEFAULT_POOL_SIZE;
		pool->cur_offset = 1;
	}

	touchdb_log(TOUCHDB_LOG_INFO, "[touchdb_pool_init]Init shared memory pool, s:%p, e:%p, size:%d",
		shm_info[pool->idx].start, shm_info[pool->idx].end, pool->size);
}

void *
touchdb_pool_malloc(touchdb_pool_t *pool, unsigned int size){
	touchdb_free_blk_list_t	*list;
	touchdb_free_blk_hdr_t	*hdr, *q;
	int						idx, blk_size, status;
	pthread_rwlock_t 		*lck;

	idx = free_list_idx(size);
	if(idx > pool->free_blk_list_count){
		touchdb_log(TOUCHDB_LOG_ERR, "[touchdb_pool_malloc]The max block size is %d, the request is too large %d",
			pool->free_blk_list_count << 3, size);
		return NULL;
	}

	list = &pool->free_blk_list[idx];
	if(list->size == 0){
		blk_size = BLK_HDR_SIZE + ((idx+1) << 3);
		if(pool->cur_offset + blk_size > pool->size){
			touchdb_log(TOUCHDB_LOG_ERR, "[touchdb_pool_malloc]The unallocated area is used up, the size of free space is %d",
				pool->size - pool->cur_offset + 1);
			return NULL;
		}

		hdr = (touchdb_free_blk_hdr_t *)cur_ptr(pool);
		hdr->idx = idx;
		set_cur_ptr_offset(pool, blk_size);
	}else{
		// get the header node and return
		hdr = free_list_header(pool, list);
		if(hdr->next_offset != NIL){
			q = blk_hdr_next(pool, hdr);
			q->prev_offset = NIL;
		}else{
			// only 1 node, set tail to NIL
			list->tail_offset = NIL;
		}
		list->header_offset = hdr->next_offset;
		list->size--;
	}

	return blk_data_by_hdr(hdr);
}

void
touchdb_pool_free(touchdb_pool_t *pool, void *p){
	touchdb_free_blk_hdr_t	*blk, *tail;
	touchdb_free_blk_list_t	*list;
	pthread_rwlock_t 		*lck;

	blk = blk_hdr_by_data(p);
	if(blk->idx < 0 || blk->idx > pool->free_blk_list_count){
		touchdb_log(TOUCHDB_LOG_ERR, "[touchdb_pool_free]The free block list dosen't exist. The length of free list is %d, and the index is %d",
			pool->free_blk_list_count, blk->idx);
		return;
	}

	list = &pool->free_blk_list[blk->idx];

	if(list->size == 0){
		blk->prev_offset = NIL;
		list->header_offset = list->tail_offset = ptr_offset_of(pool, blk);
	}else{
		tail = free_list_tail(pool, list);
		blk->prev_offset = list->tail_offset;
		tail->next_offset = ptr_offset_of(pool, blk);
		list->tail_offset = ptr_offset_of(pool, blk);
	}
	blk->next_offset = NIL;
	list->size++;
}

ptr_offset
ptr_offset_of(touchdb_pool_t *pool, void *ptr){
	if(ptr == NULL)
		return NIL;
	if((char *)ptr < (char *)shm_info[pool->idx].start){
		touchdb_log(TOUCHDB_LOG_ERR, "[ptr_offset_of]Pointer(%p) is less than pool_ptr_s(%p).",
				ptr, shm_info[pool->idx].start);
		return NIL;
	}
	return (char *)ptr - (char *)shm_info[pool->idx].start;
}

void*
ptr_of(touchdb_pool_t *pool, ptr_offset offset){
	if(offset<0 || offset > pool->size){
		touchdb_log(TOUCHDB_LOG_ERR, "[ptr_of]Offset %d is out of range, [0, %d]", offset, pool->size);
		return NULL;
	}
	return (char *)shm_info[pool->idx].start + offset;
}

void
touchdb_pool_free_blk_info(touchdb_pool_t *pool){
	touchdb_free_blk_list_t	*list;
	int						i;

	touchdb_log(TOUCHDB_LOG_ERR, "free blk list info:");
	for(i=0; i<pool->free_blk_list_count; i++){
		list = &pool->free_blk_list[i];
		if(list->size != 0){
			touchdb_log(TOUCHDB_LOG_ERR, "idx: %d, size: %d", i, list->size);
		}
	}
}
