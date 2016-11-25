/**
 * touchdb_rbtree.c
 *
 * @author chosen0ne
 * @date 2012-06-17
 *
 * Copyright (C) by chosen0ne
 */

#include "touchdb_rbtree.h"
#include "touchdb_pool.h"
#include <string.h>

extern touchdb_pool_t	*touchdb_pool;
extern touchdb_log_t	touchdb_logger;

static touchdb_rbtree_node_t* left_node(touchdb_rbtree_node_t *n);
static touchdb_rbtree_node_t* right_node(touchdb_rbtree_node_t *n);
static touchdb_rbtree_node_t* parent_node(touchdb_rbtree_node_t *n);
static touchdb_rbtree_node_t* rbtree_node(ptr_offset offset);
static void left_rotate(touchdb_rbtree_t *tree, touchdb_rbtree_node_t *n);
static void right_rotate(touchdb_rbtree_t *tree, touchdb_rbtree_node_t *n);
static touchdb_rbtree_node_t* get_node(touchdb_rbtree_t *tree, const char *k, touchdb_rbtree_node_t **parent);
static void insert_fixup(touchdb_rbtree_t *tree, touchdb_rbtree_node_t *n);
static void del_fixup(touchdb_rbtree_t *tree, touchdb_rbtree_node_t *parent, touchdb_rbtree_node_t *child);

static touchdb_rbtree_node_t*
left_node(touchdb_rbtree_node_t *n){
	if(n==NULL || n->left_offset==NIL)
		return NULL;
	return (touchdb_rbtree_node_t *)ptr_of(touchdb_pool, n->left_offset);
}

static touchdb_rbtree_node_t*
right_node(touchdb_rbtree_node_t *n){
	if(n==NULL || n->right_offset==NIL)
		return NULL;
	return (touchdb_rbtree_node_t *)ptr_of(touchdb_pool, n->right_offset);
}

static touchdb_rbtree_node_t*
parent_node(touchdb_rbtree_node_t *n){
	if(n==NULL || n->parent_offset==NIL)
		return NULL;
	return (touchdb_rbtree_node_t *)ptr_of(touchdb_pool, n->parent_offset);
}

static
touchdb_rbtree_node_t* rbtree_node(ptr_offset offset){
	if(offset == NIL)
		return NULL;
	return (touchdb_rbtree_node_t *)ptr_of(touchdb_pool, offset);
}

static touchdb_rbtree_node_t*
get_node(touchdb_rbtree_t *tree, const char *k, touchdb_rbtree_node_t **parent){
	touchdb_rbtree_node_t 	*n;
	char					*key;
	int						r;

	n = rbtree_node(tree->root_offset);
	while(n != NULL){
		key = (char *)ptr_of(touchdb_pool, n->key_offset);
		r = strcmp(k, key);
		*parent = n;
		if(r > 0)
			n = right_node(n);
		else if(r < 0)
			n = left_node(n);
		else
			return n;
	}

	return NULL;
}

static void
insert_fixup(touchdb_rbtree_t *tree, touchdb_rbtree_node_t *n){
	touchdb_rbtree_node_t	*parent, *gparent, *uncle, *tmp;

	while( (parent = parent_node(n)) && (parent->color == RED) ){
		if( (gparent = parent_node(parent)) && (parent == left_node(gparent)) ){
			uncle = right_node(gparent);
			if(uncle && uncle->color == RED){
				parent->color = BLACK;
				uncle->color = BLACK;
				gparent->color = RED;
				n = gparent;
			}else{
				if(n == right_node(parent)){
					left_rotate(tree, parent);
					tmp = parent;
					parent = n;
					n = tmp;
				}
				parent->color = BLACK;
				gparent->color = RED;
				right_rotate(tree, gparent);
			}
		}else{
			uncle = left_node(gparent);
			if(uncle && uncle->color == RED){
				uncle->color = BLACK;
				parent->color = BLACK;
				gparent->color = RED;
				n = gparent;
			}else{
				if(n == left_node(parent)){
					right_rotate(tree, parent);
					tmp = parent;
					parent = n;
					n = tmp;
				}
				parent->color = BLACK;
				gparent->color = BLACK;
				left_rotate(tree, gparent);
			}
		}
	}
	rbtree_node(tree->root_offset)->color = BLACK;
}

static void
del_fixup(touchdb_rbtree_t *tree, touchdb_rbtree_node_t *parent, touchdb_rbtree_node_t *node){
	touchdb_rbtree_node_t	*other, *o_left, *o_right;

	while( (!node || node->color == BLACK) && node != rbtree_node(tree->root_offset) ){
		if(left_node(parent) == node){
			other = right_node(parent);
			if(other->color == RED){
				other->color = BLACK;
				parent->color = RED;
				left_rotate(tree, parent);
				other = right_node(parent);
			}

			if( (left_node(other)==NULL || left_node(other)->color==BLACK) &&
				(right_node(other)==NULL || right_node(other)->color==BLACK)){
				other->color = RED;
				node = parent;
				parent = parent_node(node);
			}else{
				if(right_node(other) == NULL || right_node(other)->color == BLACK){
					if( (o_left = left_node(other)) ){
						o_left->color = BLACK;
					}
					other->color = BLACK;
					right_rotate(tree, other);
					other = right_node(parent);
				}

				other->color = parent->color;
				parent->color = BLACK;
				if(other->right_offset){
					right_node(other)->color = BLACK;
				}
				left_rotate(tree, parent);
				node = rbtree_node(tree->root_offset);
				break;
			}
		}else{
			other = left_node(parent);
			if(other->color == RED){
				other->color = BLACK;
				parent->color = RED;
				right_rotate(tree, parent);
				other = left_node(parent);
			}

			if( (left_node(other)==NULL || left_node(other)->color == BLACK) &&
				(right_node(other)==NULL || right_node(other)->color == BLACK)){
				other->color = RED;
				node = parent;
				parent = parent_node(node);
			}else{
				if(left_node(other)==NULL || left_node(other)->color==BLACK){
					if( (o_right = right_node(other)) ){
						o_right->color = BLACK;
					}
					other->color = BLACK;
					left_rotate(tree, other);
					other = left_node(parent);
				}

				other->color = parent->color;
				parent->color = BLACK;
				if(other->left_offset){
					left_node(other)->color = BLACK;
				}
				right_rotate(tree, parent);
				node = rbtree_node(tree->root_offset);
				break;
			}
		}
	}

	if(node){
		node->color = BLACK;
	}
}

static void
left_rotate(touchdb_rbtree_t *tree, touchdb_rbtree_node_t *n){
	touchdb_rbtree_node_t 	*right, *parent;
	ptr_offset				n_offset, right_offset;

	if( (right = right_node(n)) ){
		n_offset = ptr_offset_of(touchdb_pool, n);
		right_offset = ptr_offset_of(touchdb_pool, right);
		if( (n->right_offset = right->left_offset) )
			left_node(right)->parent_offset = n_offset;
		right->left_offset = n_offset;
		right->parent_offset = n->parent_offset;

		if( (parent = parent_node(n)) ){
			if(parent->right_offset == n_offset)
				parent->right_offset = right_offset;
			else
				parent->left_offset = right_offset;
		}else
			tree->root_offset = right_offset;
		n->parent_offset = right_offset;
	}
}

static void
right_rotate(touchdb_rbtree_t *tree, touchdb_rbtree_node_t *n){
	touchdb_rbtree_node_t	*left, *parent;
	int						n_offset, left_offset;

	if( (left = left_node(n)) ){
		n_offset = ptr_offset_of(touchdb_pool, n);
		left_offset = ptr_offset_of(touchdb_pool, left);

		if( (left->right_offset = left->right_offset) )
			right_node(left)->parent_offset = n_offset;
		left->right_offset = n_offset;
		left->parent_offset = n->parent_offset;

		parent = parent_node(n);
		if( (parent = parent_node(n)) ){
			if(parent->right_offset == n_offset)
				parent->right_offset = left_offset;
			else
				parent->left_offset = left_offset;
		}else
			tree->root_offset = left_offset;

		n->parent_offset = left_offset;
	}
}

void
touchdb_rbtree_insert(touchdb_rbtree_t *tree, const char *k, touchdb_val_t *v){
	touchdb_rbtree_node_t	*n, *parent;
	ptr_offset				n_offset;

	parent = NULL;
	n = get_node(tree, k, &parent);
	if(n != NULL){
		// find the node, only replace the value
		memcpy(&n->val, v, sizeof(touchdb_val_t));
	}else{
		n = (touchdb_rbtree_node_t *)touchdb_pool_malloc(touchdb_pool, sizeof(touchdb_rbtree_node_t));
		n_offset = ptr_offset_of(touchdb_pool, n);

		memcpy(&n->val, v, sizeof(touchdb_val_t));
		n->parent_offset = ptr_offset_of(touchdb_pool, parent);
		if(parent == NULL)
			tree->root_offset = n_offset;
		else{
			if(strcmp(k, (char *)ptr_of(touchdb_pool, parent->key_offset)) > 1){
				parent->right_offset = n_offset;
			}else{
				parent->left_offset = n_offset;
			}
			n->left_offset = n->right_offset = NIL;
			n->color = RED;
		}
		insert_fixup(tree, n);
	}
}

void
touchdb_rbtree_del(touchdb_rbtree_t *tree, const char *k){
	touchdb_rbtree_node_t			*n, *parent, *old, *left, *child;
	touchdb_rbtree_node_color_t		color;
	ptr_offset						n_offset;

	if( !(n = get_node(tree, k, &parent)) )
		return;
	old = n;
	if(n->left_offset && n->right_offset){
		n = right_node(n);
		while( (left = left_node(n)) != NULL)
			n = left;
		child = right_node(n);
		parent = parent_node(n);
		color = n->color;

		if(child){
			child->parent_offset = ptr_offset_of(touchdb_pool, parent);
		}
		if(parent){
			if(left_node(parent) == n)
				parent->left_offset = ptr_offset_of(touchdb_pool, child);
			else
				parent->right_offset = ptr_offset_of(touchdb_pool, child);
		}else{
			tree->root_offset = ptr_offset_of(touchdb_pool, child);
		}
		n->parent_offset = old->parent_offset;
		n->left_offset = old->left_offset;
		n->right_offset = old->right_offset;
		n->color = old->color;
		n_offset = ptr_offset_of(touchdb_pool, n);
		if(old->parent_offset){
			if(left_node(parent_node(old)) == old){
				parent_node(old)->left_offset = n_offset;
			}else{
				parent_node(old)->right_offset = n_offset;
			}
		}else
			tree->root_offset = ptr_offset_of(touchdb_pool, n);
		left_node(old)->parent_offset = n_offset;
		if(old->right_offset)
			right_node(old)->parent_offset = n_offset;
	}else{
		if(n->left_offset == NIL){
			child = right_node(n);
		}else if(n->right_offset == NIL){
			child = left_node(n);
		}
		parent = parent_node(n);
		color = n->color;

		if(child){
			child->parent_offset = ptr_offset_of(touchdb_pool, parent);
		}
		if(parent){
			if(left_node(parent) == n){
				parent->left_offset = ptr_offset_of(touchdb_pool, child);
			}else{
				parent->right_offset = ptr_offset_of(touchdb_pool, child);
			}
		}else{
			tree->root_offset = ptr_offset_of(touchdb_pool, child);
		}
	}

	touchdb_pool_free(touchdb_pool, old);
	if(color == BLACK){
		del_fixup(tree, parent, child);
	}
}


