#include "touchdb_map.h"
#include "touchdb_pool.h"
#include <string.h>

extern touchdb_log_t 	touchdb_logger;
extern touchdb_pool_t 	*touchdb_pool;

static int MAX_CAPACITY = 1 << 30;
static int MAP_ENTRY_SIZE = sizeof(touchdb_map_entry_t);

static int hash(int h);
static int index_for(touchdb_map_t *map, int h);
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

static int
index_for(touchdb_map_t *map, int h){
	return h & (map->bulk_list_count - 1);
}

static touchdb_map_bulk_t*
get_bulk(touchdb_map_t *map, int idx){
	touchdb_map_bulk_t	*bulk = (touchdb_map_bulk_t *)ptr_of(touchdb_pool, map->bulk_list_offset);
	return bulk + idx;
}

static touchdb_map_entry_t*
next_entry(touchdb_map_entry_t *entry){
	if(entry!=NULL && entry->next_offset!=NIL)
		return (touchdb_map_entry_t *)ptr_of(touchdb_pool, entry->next_offset);
	return NULL;
}

static touchdb_map_entry_t*
prev_entry(touchdb_map_entry_t *entry){
	if(entry!=NULL && entry->prev_offset!=NIL)
		return (touchdb_map_entry_t *)ptr_of(touchdb_pool, entry->prev_offset);
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
	entry = (touchdb_map_entry_t *)ptr_of(touchdb_pool, bulk->header_offset);
	while(entry != NULL){
		if(entry-)
		key = (const char *)ptr_of(touchdb_pool, entry->key_offset);
		if(strcmp(key, k) == 0)
			return entry;
		entry = next_entry(entry);
	}
	return NULL;
}

touchdb_map_t*
touchdb_map_init(touchdb_map_t *map, int capacity){
	void				*p;
	touchdb_chain_t		*bulk_list;
	int					i;

	// init capacity, make sure capacity = 2^n
	if(capacity < 0){
		touchdb_logger(TOUCHDB_LOG_ERR, "[touchdb_map_init]The capacity of map must be greater than 0");
		exit(1);
	}
	if(capacity > MAX_CAPACITY)
		capacity = MAX_CAPACITY;
	map->bulk_list_count = 1;
	while(map->bulk_list_count < capacity)
		map->bulk_list_count = map->bulk_list_count << 1;

	p = touchdb_pool_malloc(touchdb_pool, map->bulk_list_count * sizeof(touchdb_chain_t));
	map->bulk_list_offset = ptr_offset_of(touchdb_pool, p);
	bulk_list = (touchdb_chain_t *)p;

	for(i=0; i<map->bulk_list_count; i++){
		(bulk_list + i)->header_offset = NIL;
		(bulk_list + i)->tail_offset = NIL;
		(bulk_list + i)->size = 0;
	}

	touchdb_logger(TOUCHDB_LOG_INFO, "[touchdb_map_init]Map init, capacity: %d", map->bulk_list_count);
	return map;
}


const char*
touchdb_map_put(touchdb_map_t *map, const char *k, const char *v){
	touchdb_map_bulk_t		*bulk;
	touchdb_map_entry_t		*entry, *tail;
	int						h, k_len, v_len, entry_offset;
	const char				*old_val;
	char 					*new_key, *new_val;

	h = hash(hash_code(k));
	bulk = get_bulk(map, index_for(map, h));
	if(bulk->size != 0){
		entry = get_entry(map, k);
		if(entry != NULL){
			if(entry->val.type == TOUCHDB_VAL_STR)
				old_val = (const char *)ptr_of(touchdb_pool, entry->val.val_offset);
			else{
				old_val = NULL;
				entry->val.type = TOUCHDB_VAL_STR;
			}
			new_val = (char *)touchdb_pool_malloc(touchdb_pool, strlen(v) + 1);
			memcpy(new_val, v, strlen(v)+1);
			entry->val.val_offset = ptr_offset_of(touchdb_pool, new_val);
			return old_val;
		}
	}

	// init entry
	entry = (touchdb_map_entry_t *)touchdb_pool_malloc(touchdb_pool, MAP_ENTRY_SIZE);
	k_len = strlen(k) + 1;
	v_len = strlen(v) + 1;
	new_key = (char *)touchdb_pool_malloc(touchdb_pool, k_len);
	new_val = (char *)touchdb_pool_malloc(touchdb_pool, v_len);
	memcpy(new_key, k, k_len);
	memcpy(new_val, v, v_len);
	entry->key_offset = ptr_offset_of(touchdb_pool, new_key);
	entry->val.val_offset = ptr_offset_of(touchdb_pool, new_val);
	entry->val.type = TOUCHDB_VAL_STR;
	entry_offset = ptr_offset_of(touchdb_pool, entry);

	// insert entry at tail
	if(bulk->size == 0){
		bulk->header_offset = bulk->tail_offset = entry_offset;
		entry->prev_offset = NIL;
	}else{
		tail = (touchdb_map_entry_t *)ptr_of(touchdb_pool, bulk->tail_offset);
		tail->next_offset = entry_offset;
		entry->prev_offset = bulk->tail_offset;
		bulk->tail_offset = entry_offset;
	}
	entry->next_offset = NIL;
	bulk->size++;
	map->size++;
	return NULL;
}

const char*
touchdb_map_get(touchdb_map_t *map, const char *k){
	touchdb_map_entry_t 	*entry;

	entry = get_entry(map, k);
	if(entry != NULL)
		return (const char *)ptr_of(touchdb_pool, entry->val.val_offset);
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

	bulk = (touchdb_map_bulk_t *)ptr_of(touchdb_pool, map->bulk_list_offset);
	for(i=0; i<map->bulk_list_count; i++){
		if((bulk + i)->size != 0){
			entry = (touchdb_map_entry_t *)ptr_of(touchdb_pool, (bulk + i)->header_offset);
			while(entry != NULL){
				k = (const char *)ptr_of(touchdb_pool, entry->key_offset);
				v = (const char *)ptr_of(touchdb_pool, entry->val.val_offset);
				iter(k, v);

				entry = next_entry(entry);
			}
		}
	}
}

