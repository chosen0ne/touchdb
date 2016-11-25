/**
 * touchdb_rbtree.h
 *
 * @author chosen0ne
 * @date 2012-06-17
 *
 * Copyright (C) by chosen0ne
 */

#ifndef TOUCHDB_BTREE_H
#define TOUCHDB_BTREE_H

#include "touchdb_common.h"

typedef enum {
	RED,
	BLACK
} touchdb_rbtree_node_color_t;

typedef struct {
	ptr_offset 						left_offset;
	ptr_offset 						right_offset;
	ptr_offset						parent_offset;
	touchdb_rbtree_node_color_t		color;
	ptr_offset						key_offset;
	touchdb_val_t					val;
} touchdb_rbtree_node_t;

typedef struct {
	ptr_offset		root_offset;	/* touchdb_rbtree_node_t* */
	int				size;
} touchdb_rbtree_t;

typedef void (*touchdb_rbtree_iterator_t)(const char *k, touchdb_val_t *val);

void touchdb_rbtree_insert(touchdb_rbtree_t *tree, const char *k, touchdb_val_t *val);
void touchdb_rbtree_del(touchdb_rbtree_t *tree, const char *k);
void touchdb_rbtree_iter(touchdb_rbtree_t *tree, touchdb_rbtree_iterator_t iter);
int touchdb_rbtree_size();

#endif
