#include "../src/touchdb.h"
#include "../src/touchdb_rbtree.h"
#include "../src/touchdb_str.h"
#include <stdio.h>

int
main(){
	touchdb				*db;
	touchdb_rbtree_t	*rbtree;
	int					i;

	db = touchdb_new("rbtree_test.data", 20*1024*1024, 8, TOUCHDB_RBTREE, NULL);
	rbtree = &db->idx_table.rbtree;

	char* data[5] = {"a", "b", "c", "d"};
	for(i=0; i<5; i++){
		touchdb_rbtree_insert(rbtree, data[i], touchdb_str_new(data[i]));
	}
}
