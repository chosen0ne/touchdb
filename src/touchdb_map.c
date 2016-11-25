/**
 * touchdb_map.c
 *
 * @author chosen0ne
 * @date 2012-06-15
 *
 * Copyright (C) by chosen0ne
 */

#include "touchdb_map.h"
#include "touchdb_pool.h"
#include <string.h>

extern void* touchdb_malloc(touchdb_addr_t *addr, int size);
extern void touchdb_free(touchdb_addr_t *addr);
extern void* touchdb_ptr_of(touchdb_addr_t *addr);

#define index_for(map, h)		(h & (map->bulk_list_count - 1))

static int MAX_CAPACITY = 1 << 30;
static int MAP_ENTRY_SIZE = sizeof(touchdb_map_entry_t);

static int hash(int h);
static int hash_code(const char *str);
static touchdb_map_entry_t* get_entry(touchdb_map_t *map, const char *k);
static touchdb_map_bulk_t* get_bulk(touchdb_map_t *map, int idx);
static touchdb_map_entry_t* next_entry(touchdb_map_entry_t *entry);
static touchdb_map_entry_t* prev_entry(touchdb_map_entry_t *entry);

static int
hash(int h){
	h ^= (h >> 20) ^ (h >> 12);
	return h ^ (h >> 7) ^ (h >> 4);
}

static int
hash_code(const char *str){
	int h = 0;
	const char *p = str;
	while(*p != 0){
		h = 31*h + (int)*p ++;
	}
	return h;
}

static touchdb_map_bulk_t*
get_bulk(touchdb_map_t *map, int idx){
	touchdb_map_bulk_t	*bulk = (touchdb_map_bulk_t *)touchdb_ptr_of(&map->bulk_list);
	return bulk + idx;
}

static touchdb_map_entry_t*
next_entry(touchdb_map_entry_t *entry){
	if(entry!=NULL && entry->next.offset!=NIL)
		return (touchdb_map_entry_t *)touchdb_ptr_of(&entry->next);
	return NULL;
}

static touchdb_map_entry_t*
prev_entry(touchdb_map_entry_t *entry){
	if(entry!=NULL && entry->prev.offset!=NIL)
		return (touchdb_map_entry_t *)touchdb_ptr_of(&entry->prev);
	return NULL;
}

static touchdb_map_entry_t*
get_entry(touchdb_map_t *map, const char *k){
	touchdb_map_entry_t	*entry;
	touchdb_map_bulk_t	*bulk;
	int					h;
	const char			*key;

	h = hash(hash_code(k));
	bulk = get_bulk(map, index_for(map, h));
	entry = (touchdb_map_entry_t *)touchdb_ptr_of(&bulk->header);
	while(entry != NULL){
		if(entry->hash == h){	// �ȱȽ�hashֵ��hashֵ����ٽ����ַ�Ƚ�
			key = (const char *)touchdb_ptr_of(&entry->key);
			if(strcmp(key, k) == 0)
				return entry;
		}
		entry = next_entry(entry);
	}

	return NULL;
}

touchdb_map_t*
touchdb_map_init(touchdb_map_t *map, int capacity, int load_factor){
	void				*p;
	touchdb_map_bulk_t	*bulk_list;
	int					i;

	// init capacity, make sure capacity = 2^n
	if(capacity < 0){
		touchdb_log(TOUCHDB_LOG_ERR, "[touchdb_map_init]The capacity of map must be greater than 0");
		exit(1);
	}

	if(capacity > MAX_CAPACITY)
		capacity = MAX_CAPACITY;

	map->bulk_list_count = 1;
	while(map->bulk_list_count < capacity)
		map->bulk_list_count = map->bulk_list_count << 1;

	p = touchdb_malloc(&map->bulk_list, map->bulk_list_count * sizeof(touchdb_map_bulk_t));
	bulk_list = (touchdb_map_bulk_t *)p;

	for(i=0; i<map->bulk_list_count; i++){
		clear_addr(bulk_list[i].header);
		clear_addr(bulk_list[i].tail);
		bulk_list[i].size = 0;
	}

	touchdb_log(TOUCHDB_LOG_INFO, "[touchdb_map_init]Map init, capacity: %d", map->bulk_list_count);

	return map;
}


const char*
touchdb_map_put(touchdb_map_t *map, const char *k, const char *v){
	touchdb_map_bulk_t		*bulk;
	touchdb_map_entry_t		*entry, *tail;
	int						h, k_len, v_len;
	touchdb_val_t			*old_val;
	char 					*new_key, *new_val;
	touchdb_addr_t			new_addr;

	h = hash(hash_code(k));
	bulk = get_bulk(map, index_for(map, h));

	if(bulk->size != 0){
		entry = get_entry(map, k);
		if(entry != NULL){
			if(entry->val.type == TOUCHDB_VAL_STR)
				old_val = &entry->val;
			else{
				old_val = NULL;
				entry->val.type = TOUCHDB_VAL_STR;
			}

			new_val = (char *)touchdb_malloc(&new_addr, strlen(v) + 1);
			memcpy(new_val, v, strlen(v)+1);

			entry->val.value = new_addr;
			if(old_val != NULL){
				touchdb_free(&old_val->value);
				return touchdb_ptr_of(&old_val->value);
			}

			return NULL;
		}
	}

	// init entry
	entry = (touchdb_map_entry_t *)touchdb_malloc(&new_addr, MAP_ENTRY_SIZE);
	entry->hash = h;

	k_len = strlen(k) + 1;
	v_len = strlen(v) + 1;
	new_key = (char *)touchdb_malloc(&entry->key, k_len);
	new_val = (char *)touchdb_malloc(&entry->val.value, v_len);
	memcpy(new_key, k, k_len);
	memcpy(new_val, v, v_len);

	entry->val.type = TOUCHDB_VAL_STR;

	// insert entry at tail
	if(bulk->size == 0){
		bulk->header = bulk->tail = new_addr;
		entry->prev = NULL_ADDR;
	}else{
		tail = (touchdb_map_entry_t *)touchdb_ptr_of(&bulk->tail);
		tail->next = new_addr;
		entry->prev = bulk->tail;
		bulk->tail = new_addr;
	}

	entry->next = NULL_ADDR;
	bulk->size++;
	map->size++;

	return NULL;
}

const char*
touchdb_map_get(touchdb_map_t *map, const char *k){
	touchdb_map_entry_t 	*entry;

	entry = get_entry(map, k);
	if(entry != NULL)
		return (const char *)touchdb_ptr_of(&entry->val.value);
	return NULL;
}

const touchdb_val_t*
touchdb_map_put2(touchdb_map_t *map, const char *k, const touchdb_val_t *v){
	touchdb_map_bulk_t		*bulk;
	touchdb_map_entry_t		*entry, *tail;
	int						h, k_len;
	const touchdb_val_t		*old_val;
	char 					*new_key;
	touchdb_addr_t			new_addr;

	h = hash(hash_code(k));
	bulk = get_bulk(map, index_for(map, h));

	if(bulk->size != 0){
		entry = get_entry(map, k);
		if(entry != NULL){
			old_val = &entry->val;

			entry->val = *v;

			touchdb_val_destroy((touchdb_val_t *)old_val);

			return old_val;
		}
	}

	// init entry
	entry = (touchdb_map_entry_t *)touchdb_malloc(&new_addr, MAP_ENTRY_SIZE);
	entry->hash = h;
	entry->val = *v;

	k_len = strlen(k) + 1;
	new_key = touchdb_malloc(&entry->key, k_len);
	memcpy(new_key, k, k_len);

	// insert entry at tail
	if(bulk->size == 0){
		bulk->header = bulk->tail = new_addr;
		entry->prev = NULL_ADDR;
	}else{
		tail = (touchdb_map_entry_t *)touchdb_ptr_of(&bulk->tail);
		tail->next = new_addr;
		entry->prev = bulk->tail;
		bulk->tail = new_addr;
	}

	entry->next = NULL_ADDR;
	bulk->size++;
	map->size++;

	return NULL;
}

const touchdb_val_t*
touchdb_map_get2(touchdb_map_t *map, const char *k){
	touchdb_map_entry_t 	*entry;

	entry = get_entry(map, k);
	if(entry != NULL)
		return &entry->val;
	return NULL;
}

const char*
touchdb_map_del(touchdb_map_t *map, const char *k){

}

bool
touchdb_map_contains(touchdb_map_t *map, const char *k){
	touchdb_map_entry_t 	*entry;

	entry = get_entry(map, k);
	return entry != NULL;
}

int
touchdb_map_size(touchdb_map_t *map){
	return map->size;
}

void
touchdb_map_iter(touchdb_map_t *map, touchdb_map_iterator_t iter){
	touchdb_map_bulk_t 	*bulk;
	touchdb_map_entry_t	*entry;
	int					i;
	const char			*k, *v;

	bulk = (touchdb_map_bulk_t *)touchdb_ptr_of(&map->bulk_list);
	for(i=0; i<map->bulk_list_count; i++){

		if((bulk + i)->size != 0){
			entry = (touchdb_map_entry_t *)touchdb_ptr_of(&(bulk + i)->header);

			while(entry != NULL){
				k = (const char *)touchdb_ptr_of(&entry->key);
				v = (const char *)touchdb_ptr_of(&entry->val.value);
				iter(k, v);

				entry = next_entry(entry);
			}
		}
	}
}

int
touchdb_map_cmp(touchdb_map_t *m1, touchdb_map_t *m2){

}

void
touchdb_map_destory(touchdb_map_t *m){

}
