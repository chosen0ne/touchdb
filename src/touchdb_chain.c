/**
 * touchdb_chain.c
 *
 * @author chosen0ne
 * @date 2012-06-14
 *
 * Copyright (C) by chosen0ne
 */

#include<string.h>
#include "touchdb_chain.h"

const int TOUCHDB_CHAIN_NODE_SIZE = sizeof(touchdb_chain_node_t);

static touchdb_chain_node_t* touchdb_chain_node_new(touchdb_addr_t *addr,
		touchdb_addr_t *next, touchdb_addr_t *prev, touchdb_val_t *val);
static void touchdb_chain_node_destroy(touchdb_chain_node_t *n);
static touchdb_chain_node_t* next_node(touchdb_chain_node_t *n);
static touchdb_chain_node_t* prev_node(touchdb_chain_node_t *n);
static touchdb_chain_node_t* chain_header(touchdb_chain_t *chain);
static touchdb_chain_node_t* chain_tail(touchdb_chain_t *chain);

static touchdb_chain_node_t*
touchdb_chain_node_new(touchdb_addr_t *addr, touchdb_addr_t *next,
		touchdb_addr_t *prev, touchdb_val_t *val){
	touchdb_chain_node_t	*n;

	if( (n = touchdb_malloc(addr, TOUCHDB_CHAIN_NODE_SIZE)) == NULL ){
		touchdb_log(TOUCHDB_LOG_ERR, "[touchdb_chain_node_new]Can't allocate memory for touchdb_chain_node_t");
		return NULL;
	}

	if(next == NULL)
		n->next = NULL_ADDR;
	else
		n->next = *next;

	if(prev == NULL)
		n->prev = NULL_ADDR;
	else
		n->prev = *prev;

	n->val = *val;

	return n;
}

static void
touchdb_chain_node_destroy(touchdb_chain_node_t *n){
	touchdb_addr_t	addr;
	if(n != NULL){
		touchdb_val_destroy(&n->val);
		touchdb_addr_of(&addr, n);
		touchdb_free(&addr);
	}
}

static touchdb_chain_node_t*
next_node(touchdb_chain_node_t *n){
	if(n == NULL || n->next.offset == NIL)
		return NULL;
	return touchdb_ptr_of(&n->next);
}

static touchdb_chain_node_t*
prev_node(touchdb_chain_node_t *n){
	if(n == NULL || n->prev.offset == NIL)
		return NULL;
	return touchdb_ptr_of(&n->prev);
}

static touchdb_chain_node_t*
chain_header(touchdb_chain_t *chain){
	if(chain == NULL || chain->header.offset == NIL)
		return NULL;
	return touchdb_ptr_of(&chain->header);
}

static touchdb_chain_node_t*
chain_tail(touchdb_chain_t *chain){
	if(chain == NULL || chain->tail.offset == NIL)
		return NULL;
	return touchdb_ptr_of(&chain->tail);
}

touchdb_chain_t*
touchdb_chain_init(touchdb_chain_t *chain){
	memset(chain, 0, sizeof(touchdb_chain_t));
	touchdb_log(TOUCHDB_LOG_INFO, "[touchdb_chain_init]Chain init.");

	return chain;
}

touchdb_chain_t*
touchdb_chain_new(touchdb_val_t *val){
	touchdb_chain_t		*chain;

	chain = touchdb_malloc(&val->value, sizeof(touchdb_chain_t));
	val->type = TOUCHDB_VAL_CHAIN;
	touchdb_chain_init(chain);

	return chain;
}

void
touchdb_chain_push_head(touchdb_chain_t *chain, touchdb_val_t *val){
	touchdb_chain_node_t 	*head, *n;
	touchdb_addr_t			addr;

	n = touchdb_chain_node_new(&addr, &chain->header, NULL, val);

	if(chain->size == 0){
		chain->header = chain->tail = addr;
	}else{
		head = chain_header(chain);
		chain->header = head->next = addr;
	}
	chain->size++;
}

void
touchdb_chain_push_tail(touchdb_chain_t *chain, touchdb_val_t *val){
	touchdb_chain_node_t	*tail, *n;
	touchdb_addr_t			addr;

	n = touchdb_chain_node_new(&addr, NULL, &chain->tail, val);
	if(chain->size == 0){
		chain->header = chain->tail = addr;
	}else{
		tail = chain_tail(chain);
		chain->tail = tail->next = addr;
	}

	chain->size++;
}

const touchdb_val_t*
touchdb_chain_pull_head(touchdb_chain_t *chain){
	touchdb_chain_node_t	*head;
	touchdb_val_t			*val;

	head = chain_header(chain);
	if(head == NULL)
		return NULL;
	chain->header = head->next;
	if(chain->header.offset == NIL)
		chain->tail.offset = NIL;
	chain->size--;

	/**
	 * need 'touchdb_val_cpy'
	 */
	val = touchdb_malloc(NULL, sizeof(touchdb_val_t));
	memcpy(val, &head->val, sizeof(touchdb_val_t));
	touchdb_chain_node_destroy(head);

	return val;
}

const touchdb_val_t*
touchdb_chain_pull_tail(touchdb_chain_t *chain){
	touchdb_chain_node_t	*tail;
	touchdb_val_t			*val;

	tail = chain_tail(chain);
	if(tail == NULL)
		return NULL;
	chain->tail = tail->prev;
	if(chain->tail.offset == NIL)
		chain->header.offset = NIL;
	chain->size--;

	val = touchdb_malloc(NULL, sizeof(touchdb_val_t));
	memcpy(val, &tail->val, sizeof(touchdb_val_t));
	touchdb_chain_node_destroy(tail);

	return val;
}

void
touchdb_chain_insert(touchdb_chain_t *chain, touchdb_chain_node_t *n, touchdb_val_t *val){
	touchdb_chain_node_t	*node, *next_n;
	touchdb_addr_t			addr, naddr;

	if(n == NULL)
		return;

	// get addr of n
	touchdb_addr_of(&naddr, n);
	node = touchdb_chain_node_new(&addr, &n->next, &naddr, val);
	next_n = next_node(n);
	next_n->prev = addr;
	n->next = addr;
	chain->size++;
}

void touchdb_chain_insert_idx(touchdb_chain_t *chain, int idx, touchdb_val_t *val){
	touchdb_chain_node_t	*n;

	n = (touchdb_chain_node_t *)touchdb_chain_node_idx(chain, idx);
	touchdb_chain_insert(chain, n, val);
}

int
touchdb_chain_index_of(touchdb_chain_t *chain, touchdb_val_t *val){
	touchdb_chain_node_t	*n;
	int						idx;

	for(idx=0, n=chain_header(chain); n!=NULL; idx++, n=next_node(n)){
		if(touchdb_val_cmp(&n->val, val) == 0)
			return idx;
	}
	return -1;
}

const touchdb_val_t*
touchdb_chain_node_idx(touchdb_chain_t *chain, int idx){
	touchdb_chain_node_t	*n;
	int						i;

	for(n=chain_header(chain), i=0; n!=NULL && i<idx-1; i++, n=next_node(n))
		;

	if(n != NULL && i == idx-1)
		return &n->val;

	return NULL;
}

const touchdb_chain_node_t*
touchdb_chain_header(touchdb_chain_t *chain){
	return chain_header(chain);
}

const touchdb_chain_node_t*
touchdb_chain_tail(touchdb_chain_t *chain){
	return chain_tail(chain);
}

const touchdb_chain_node_t*
touchdb_chain_node(touchdb_chain_t *chain, touchdb_val_t *val){
	touchdb_chain_node_t	*n;

	for(n=chain_header(chain); n!=NULL; n=next_node(n)){
		if(touchdb_val_cmp(&n->val, val) == 0)
			return n;
	}
	return NULL;
}

touchdb_array_t*
touchdb_chain_range(touchdb_chain_t *chain, int s, int e){
	touchdb_array_t			*array;
	touchdb_chain_node_t	*n;
	int						i;

}

int
touchdb_chain_size(touchdb_chain_t *chain){
	return chain->size;
}

const touchdb_chain_node_t*
touchdb_chain_del(touchdb_chain_t *chain, touchdb_val_t *val){
	touchdb_chain_node_t	*n, *p, *q;

	n = (touchdb_chain_node_t *)touchdb_chain_node(chain, val);
	if(n == NULL)
		return NULL;

	p = prev_node(n);
	q = next_node(n);
	if(p != NULL)
		p->next = n->next;
	if(q != NULL)
		q->prev = n->prev;

	if( n == touchdb_chain_header(chain) )
		chain->header = n->next;
	if( n == touchdb_chain_tail(chain) )
		chain->tail = n->prev;

	chain->size--;
	return n;
}

bool
touchdb_chain_contains(touchdb_chain_t *chain, touchdb_val_t *val){
	return touchdb_chain_node(chain, val) != NULL;
}

void
touchdb_chain_destory(touchdb_chain_t *chain){
	touchdb_chain_node_t	*head, *p;

	head = chain_header(chain);
	while(head != NULL){
		p = head;
		head = next_node(head);
		touchdb_chain_node_destroy(p);
	}
}

void
touchdb_chain_iter(touchdb_chain_t *chain, touchdb_chain_iterator_t iter){
	touchdb_chain_node_t 	*n;
	int						idx;

	for(n=chain_header(chain); n!=NULL; n=next_node(n)){
		iter(NULL, &n->val);
	}
}

int
touchdb_chain_cmp(touchdb_val_t *v1, touchdb_val_t *v2){
	touchdb_chain_node_t	*p, *q;
	touchdb_chain_t			*c1, *c2;
	int						r;

	c1 = touchdb_ptr_of(&v1->value);
	c2 = touchdb_ptr_of(&v2->value);

	for(p=chain_header(c1), q=chain_header(c2); p!=NULL&&q!=NULL;
			p=next_node(p), q=next_node(q)){
		r = touchdb_val_cmp(&p->val, &q->val);
		if(r != 0)
			return r;
	}
	if(p != NULL)
		return 1;
	else if(q != NULL)
		return -1;
	return 0;
}
