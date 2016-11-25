/**
 * touchdb_chain.h
 *
 * @author chosen0ne
 * @date 2012-06-14
 *
 * Copyright (C) by chosen0ne
 */

#ifndef TOUCHDB_CHAIN_H
#define TOUCHDB_CHAIN_H

#include "touchdb_common.h"
#include "touchdb_array.h"
#include <pthread.h>

typedef struct {
	touchdb_addr_t	prev;
	touchdb_addr_t 	next;
	touchdb_val_t 	val;
} touchdb_chain_node_t;

typedef struct {
	touchdb_addr_t	header;
	touchdb_addr_t	tail;
	int				size;
} touchdb_chain_t;

typedef void (*touchdb_chain_iterator_t)(const char *k, touchdb_val_t *v);

touchdb_chain_t* touchdb_chain_init(touchdb_chain_t *chain);
touchdb_chain_t* touchdb_chain_new(touchdb_val_t *val);

void touchdb_chain_push_head(touchdb_chain_t *chain, touchdb_val_t *val);
void touchdb_chain_push_tail(touchdb_chain_t *chain, touchdb_val_t *val);

const touchdb_val_t* touchdb_chain_pull_head(touchdb_chain_t *chain);
const touchdb_val_t* touchdb_chain_pull_tail(touchdb_chain_t *chain);

void touchdb_chain_insert(touchdb_chain_t *chain, touchdb_chain_node_t *n, touchdb_val_t *val);
void touchdb_chain_insert_idx(touchdb_chain_t *chain, int idx, touchdb_val_t *val);

int touchdb_chain_index_of(touchdb_chain_t *chain, touchdb_val_t *val);
const touchdb_val_t* touchdb_chain_node_idx(touchdb_chain_t *chain, int idx);

const touchdb_chain_node_t* touchdb_chain_header(touchdb_chain_t *chain);
const touchdb_chain_node_t* touchdb_chain_tail(touchdb_chain_t *chain);

const touchdb_chain_node_t* touchdb_chain_node(touchdb_chain_t *chain, touchdb_val_t *val);
touchdb_array_t* touchdb_chain_range(touchdb_chain_t *chain, int s, int e);


int touchdb_chain_size(touchdb_chain_t *chain);
const touchdb_chain_node_t* touchdb_chain_del(touchdb_chain_t *chain, touchdb_val_t *val);

bool touchdb_chain_contains(touchdb_chain_t *chain, touchdb_val_t *val);
void touchdb_chain_destory(touchdb_chain_t *chain);
void touchdb_chain_iter(touchdb_chain_t *chain, touchdb_chain_iterator_t iter);
int touchdb_chain_cmp(touchdb_val_t *c1, touchdb_val_t *c2);

#endif
