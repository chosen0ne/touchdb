/**
 * touchdb.c
 *
 * @author chosen0ne
 * @date 2012-06-10
 *
 * Copyright (C) by chosen0ne
 */

#include "touchdb.h"
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define FILE_MODE   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

extern void* ptr_of(touchdb_pool_t *pool, ptr_offset offset);

touchdb_log_t			touchdb_log = touchdb_default_logger;
touchdb					*db;

/*
 * Shared memory information of each pool, include begin and end address.
 * As the shared memory mapped at different part of process's address space,
 * the shared memory information is private to process.
 */
touchdb_shm_info_t		shm_info[TOUCHDB_POOL_LIST_SIZE];

static void make_sure_dir(const char *dat_path, char *path);
static void* get_shm(const char *file, int size);
static touchdb_chain_t* get_chain(touchdb *db, const char *);

static void
make_sure_dir(const char *dat_path, char *path){
	char 	*tmp_path;

	if(*dat_path != '/'){
		// relative path, change it to absolute path

		if(*dat_path == '.'){
			tmp_path = (char *)dat_path + 2;
		}else{
			tmp_path = (char *)dat_path;
		}

		if(getcwd(path, 1024) == NULL){
			touchdb_log(TOUCHDB_LOG_ERR, "[touchdb_new]getcwd() error. msg: %s",
				strerror(errno));
			exit(1);
		}

		strcat(path, "/");
		strcat(path, tmp_path);
		//touchdb_strcat(path, "/", tmp_path);
	}else{
		strcpy(path, dat_path);
	}

	if( *(path + strlen(path) -1) != '/'){
		strcpy(path, "/");
	}

	// directory isn't exist, create it
	if( access(path, F_OK) != 0 ){
		if( mkdir(path, 0777) == -1 ){
			touchdb_log(TOUCHDB_LOG_ERR, "[make_sure_dir]mkdir() error. msg: %s",
					strerror(errno));
			exit(1);
		}
	}
}

static void*
get_shm(const char *file, int size){
	int 			fd;
	void 			*ptr;
	struct stat 	buf;

	if( (fd = open(file, O_RDWR | O_CREAT, FILE_MODE)) == -1 ){
		touchdb_log(TOUCHDB_LOG_ERR, "[get_shm]Open data file error. msg: %s, path: %s",
			strerror(errno), file);
		return NULL;
	}

	// make sure the length of data file equals to size
	fstat(fd, &buf);
	if((int)buf.st_size < size){
		lseek(fd, size, SEEK_SET);
		write(fd, " ", 1);
	}

	if( (ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL ){
		touchdb_log(TOUCHDB_LOG_ERR, "[get_shm]mmap error. msg: %s", strerror(errno));
		return NULL;
	}

	if(close(fd) == -1){
		touchdb_log(TOUCHDB_LOG_ERR, "[get_shm]Close file error. msg: %s", strerror(errno));
		return NULL;
	}
	return ptr;
}

static touchdb_chain_t*
get_chain(touchdb *db, const char *k){
	const touchdb_val_t		*val;
	touchdb_chain_t			*chain;

	val = touchdb_map_get2(&db->idx_table.map, k);
	if( val != NULL && val->type == TOUCHDB_VAL_CHAIN ){

		chain = touchdb_ptr_of((touchdb_addr_t *)&val->value);

		if(chain != NULL)
			return chain;

	}

	return NULL;
}

touchdb*
touchdb_new(const char *dat_path, int lock_granularity, touchdb_mode mode,
		touchdb_log_t log_handler){
	bool 			is_inited, pool_is_inited;
	void			*info_ptr, *pool_ptr;
	char			path[1024], *tmp_path;
	touchdb			*tchdb;
	touchdb_pool_t	*touchdb_pool;
	int				i = 0;

	// init logger
	touchdb_log = log_handler;
	if(touchdb_log == NULL){
		touchdb_log = touchdb_default_logger;
	}

	if(dat_path == NULL){
		// default data path is current directory
		dat_path = "./";
	}

	// change relative path to absolute path
	make_sure_dir(dat_path, path);
	tmp_path = malloc(strlen(path));
	strcpy(tmp_path, path);

	// test info file exists or not
	strcat(path, TOUCHDB_INFO_FILE);
	is_inited = false;
	if(access(path, F_OK) == 0){
		is_inited = true;
	}

	info_ptr = get_shm(path, TOUCHDB_INFO_FILE_SIZE);
	db = tchdb = (touchdb *)info_ptr;
	strcpy(db->db_path, tmp_path);
	free(tmp_path);

	if(is_inited){
		// load data from data file if it exists

		if(strncmp(tchdb->magic_num, "TOUCHDB", 7) != 0){
			// data file is invalid
			touchdb_log(TOUCHDB_LOG_ERR, "Data file is invalid");
			exit(1);
		}

		// load all shared memory pool
		touchdb_pool = tchdb->pool_list;
		while( *touchdb_pool->shm_file != 0 ){
			// open shared memory
			touchdb_strcpy(path, db->db_path, touchdb_pool->shm_file, NULL);
			pool_ptr = get_shm(path, TOUCHDB_DEFAULT_POOL_SIZE);
			touchdb_pool_init(touchdb_pool, pool_ptr, lock_granularity, true);
			touchdb_pool->idx = i++;
			touchdb_pool++;
		}
	}else{
		// init touchdb

		touchdb_strcpy(tchdb->magic_num, "TOUCHDB", TOUCHDB_VERSION, NULL);
		tchdb->mode = mode;

		tchdb->lock_granularity = lock_granularity;
		memset(&tchdb->pool_list, 0, sizeof(touchdb_pool_t) * TOUCHDB_POOL_LIST_SIZE);
		touchdb_pool = tchdb->pool_list;
		touchdb_pool->idx = 0;

		// shm file is 'touchdb1.dat'
		touchdb_strcpy(touchdb_pool->shm_file, TOUCHDB_DATA_FILE_PREFIX, "1",
				TOUCHDB_DATA_FILE_SUFFIX, NULL);

		// get shared memory
		touchdb_strcpy(path, db->db_path, touchdb_pool->shm_file, NULL);
		pool_ptr = get_shm(path, TOUCHDB_DEFAULT_POOL_SIZE);
		touchdb_pool_init(touchdb_pool, pool_ptr, lock_granularity, false);

		switch(mode){
			case TOUCHDB_MAP:
				touchdb_map_init(&tchdb->idx_table.map, 1000, 0.5);
				break;
			case TOUCHDB_CHAIN:
				//touchdb_chain_init(&tchdb->idx_table.chain);
				break;
		}
	}
	return tchdb;
}

touchdb*
touchdb_new2(const char *config, touchdb_log_t log_handler){

}

void
touchdb_default_logger(int level, const char *fmt, ...){
	char		buf[1024];
	va_list		ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	printf("%s\n", buf);
}

void*
touchdb_malloc(touchdb_addr_t *addr, int size){
	touchdb_pool_t			*pool;
	void					*p, *pool_ptr;
	int						i;
	char					*seq;

	// iterate the pool list
	pool = db->pool_list;
	i = 1;
	while(pool->shm_file != NULL){
		p = touchdb_pool_malloc(pool, size);
		if(p != NULL){
			if(addr != NULL){
				set_addr(addr, ptr_offset_of(pool, p), i-1);
			}

			return p;
		}

		p++;
		i++;
	}

	// there isn't free space, create a new shared memory pool
	seq = "0";
	*seq = '0' + i;
	touchdb_strcpy(pool->shm_file, TOUCHDB_DATA_FILE_PREFIX, seq,
			TOUCHDB_DATA_FILE_SUFFIX, NULL);
	pool_ptr = get_shm(pool->shm_file, TOUCHDB_DEFAULT_POOL_SIZE);
	touchdb_pool_init(pool, pool_ptr, db->lock_granularity, false);

	if( (p = touchdb_pool_malloc(pool, size)) == NULL){
		touchdb_log(TOUCHDB_LOG_ERR, "[touchdb_malloc]Create new shared memory pool error");
		return NULL;
	}

	if(addr != NULL){
		set_addr(addr, ptr_offset_of(pool, p), i-1);
	}

	return p;
}

void
touchdb_free(touchdb_addr_t *addr){
	touchdb_pool_t			*pool;

	pool = &db->pool_list[addr->pool_idx];

	touchdb_pool_free(pool, ptr_of(pool, addr->offset));

	/**
	 * TO DO: when the pool is empty, it must be recycled and the file must be
	 * removed.
	 */
}

void*
touchdb_ptr_of(touchdb_addr_t *addr){
	return ptr_of(&db->pool_list[addr->pool_idx], addr->offset);
}

void
touchdb_addr_of(touchdb_addr_t *addr, void *p){
	touchdb_pool_t		*pool;
	int					i = 0;

	pool = db->pool_list;
	while(pool->cur_offset != NIL){
		if( ((char *)shm_info[pool->idx].start <= (char *)p) &&
				((char *)p <= (char *)shm_info[pool->idx].end) ){
			addr->pool_idx = i;
			addr->offset = (char *)p - (char *)shm_info[pool->idx].start;

			return;
		}

		i++;
		pool++;
	}

	touchdb_log(TOUCHDB_LOG_ERR, "[touchdb_addr_of]Address %p dosen't exist in any pool.",
			p);
	exit(1);
}

const char*
touchdb_put(touchdb *db, const char *k, const char *v){
	touchdb_map_t *map;

	map = (touchdb_map_t *)&db->idx_table.map;
	return touchdb_map_put(map, k, v);
}

const char*
touchdb_get(touchdb *db, const char *k){
	touchdb_map_t *map;

	map = (touchdb_map_t *)&db->idx_table.map;
	return touchdb_map_get(map, k);
}

int
touchdb_size(touchdb *db){
	touchdb_map_t *map;

	map = (touchdb_map_t *)&db->idx_table.map;
	return touchdb_map_size(map);
}

void
touchdb_iter(touchdb *db, touchdb_map_iterator_t iter){
	touchdb_map_t *map;

	map = (touchdb_map_t *)&db->idx_table.map;
	touchdb_map_iter(map, iter);
}

const touchdb_val_t*
touchdb_put_str(touchdb *db, const char *k, const char *v){
	touchdb_val_t	*val;
	touchdb_str_t 	*str;
	touchdb_addr_t	addr;

	val = touchdb_malloc(&addr, sizeof(touchdb_val_t));
	touchdb_str_new2(val, v);

	val = (touchdb_val_t *)touchdb_map_put2(&db->idx_table.map, k, val);
	touchdb_free(&addr);

	return val;
}

const char*
touchdb_get_str(touchdb *db, const char *k){
	const touchdb_val_t		*val;
	touchdb_str_t			*s;

	val = touchdb_map_get2(&db->idx_table.map, k);
	if(val != NULL && val->type == TOUCHDB_VAL_STR){
		s = touchdb_ptr_of((touchdb_addr_t *)&val->value);

		return touchdb_ptr_of(&s->data);
	}

	return NULL;
}

void
touchdb_list_push(touchdb *db, const char *k, touchdb_val_t *v){
	touchdb_val_t		*val;
	touchdb_chain_t		*chain;
	touchdb_addr_t		addr;

	if( (chain = get_chain(db, k)) != NULL) {
		touchdb_chain_push_tail(chain, v);
		return;
	}

	val = touchdb_malloc(&addr, sizeof(touchdb_val_t));
	chain = touchdb_chain_new(val);

	touchdb_chain_push_tail(chain, v);
	touchdb_map_put2(&db->idx_table.map, k, val);

	touchdb_free(&addr);
}

const touchdb_val_t*
touchdb_list_pop(touchdb *db, const char *k){
	const touchdb_val_t			*val;
	touchdb_chain_t				*chain;
	const touchdb_chain_node_t	*n;

	if( (chain = get_chain(db, k)) != NULL ){
		val = touchdb_chain_pull_head(chain);

		return val;
	}

	return NULL;
}

const touchdb_val_t*
touchdb_list_node_idx(touchdb *db, const char *k, int idx){
	const touchdb_val_t		*val;
	touchdb_chain_t			*chain;

	if( (chain = get_chain(db, k)) != NULL ){
		val = touchdb_chain_node_idx(chain, idx);

		return val;
	}

	return NULL;
}

int
touchdb_list_size(touchdb *db, const char *k){
	const touchdb_val_t		*val;
	touchdb_chain_t			*chain;

	if( (chain = get_chain(db, k)) != NULL )
		return touchdb_chain_size(chain);

	return -1;
}

/*
int
main(int argc, char **argv){
	touchdb 		*db;
	touchdb_chain_t	*chain;
	touchdb_val_t	*val;
	int				i;

	db = touchdb_new(NULL, 1024*1024*20, 8, TOUCHDB_CHAIN, NULL);
	chain = &db->idx_table.chain;

	/*char* str[3] = {"abc", "qew", "123"};
	for(i=0; i<3; i++){
		val = touchdb_str_new(str[i]);
		touchdb_chain_push_head(chain, val);
	}

	touchdb_chain_iter(chain, chain_print);
}
*/
