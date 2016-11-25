#include "../src/touchdb.h"
#include "../src/touchdb_chain.h"
#include "../src/touchdb_str.h"
#include <stdio.h>

void
chain_print(const char *k, touchdb_val_t *v){
	if(v->type = TOUCHDB_VAL_STR)
		printf("%s, ", touchdb_str_data(v));
}

int
main(){
	touchdb				*db;
	int					i;
	touchdb_chain_t		*chain;

	db = touchdb_new("chain_test.data", 20*1024*1024, 8, TOUCHDB_CHAIN, NULL);
	char* data[5] = {"123", "abc", "123bca", "345aewdqwe", "123123fasdf"};
	chain = &db->idx_table.chain;

	for(i=0; i<5; i++){
		touchdb_chain_push_tail(chain, touchdb_str_new(data[i]));
	}
	touchdb_chain_iter(chain, chain_print);
	printf("size: %d\n", chain->size);

	const touchdb_chain_node_t *h = touchdb_chain_header(chain);
	touchdb_val_t *v = (touchdb_val_t *)&h->val;
	printf("header: %s\n", touchdb_str_data(v));
	touchdb_chain_push_tail(chain, touchdb_str_new("nba"));
	touchdb_chain_iter(chain, chain_print);
	printf("size: %d\n", chain->size);

	v = touchdb_str_new("abc");
	int idx = touchdb_chain_index_of(chain, v);
	printf("node 'abc' idx: %d\n", idx);
	touchdb_chain_del(chain, v);
	touchdb_chain_iter(chain, chain_print);
	printf("size: %d\n", chain->size);

	v= (touchdb_val_t *)&touchdb_chain_node_idx(chain, 3)->val;
	printf("3 node: %s\n", touchdb_str_data(v));
	v = touchdb_str_new("nvnvnvn");
	touchdb_chain_insert_idx(chain, 3, v);
	touchdb_chain_iter(chain, chain_print);
	printf("size: %d\n", chain->size);
}
