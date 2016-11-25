/**
 * touchdb_worker.c
 *
 * @author chosen0ne
 * @date 2012-06-23
 *
 * Copyright (C) by chosen0ne
 */

#include <stdio.h>
#include <stdbool.h>
#include "touchdb.h"

void print_iter(const char *k, const char *v){
	printf("%s => %s\n", k, v);
}

int
main(int argc, char **argv){
	touchdb 		*db;
	int				cmd, idx;
	char			k[100], v[100];
	touchdb_addr_t	addr;
	touchdb_val_t	*val_a;
	touchdb_str_t	*s;

	db = touchdb_new("./touchdb/", 8, TOUCHDB_MAP, NULL);
	while(true){
		printf("touchdb size: %d, operation: 1.put, 2.get, 3.iter, 4.list push, 5.list pop, "
				"6.list index, 7.list size, -1.exit\n", touchdb_size(db));
		scanf("%d", &cmd);
		if(cmd == -1)
			exit(0);
		switch(cmd){
			case 1:
				printf("key: ");
				scanf("%s", k);
				printf("value: ");
				scanf("%s", v);
				//touchdb_put(db, k, v);
				touchdb_put_str(db, k, v);
				break;
			case 2:
				printf("key: ");
				scanf("%s", k);
				//printf("(%s) => (%s)\n", k, touchdb_get(db, k));
				printf("(%s) => (%s)\n", k, touchdb_get_str(db, k));
				break;
			case 3:
				touchdb_iter(db, print_iter);
				break;
			case 4:
				printf("key: ");
				scanf("%s", k);
				printf("value: ");
				scanf("%s", v);
				val_a = touchdb_malloc(&addr, sizeof(touchdb_val_t));
				s = touchdb_str_new2(val_a, v);
				touchdb_list_push(db, k, val_a);
				touchdb_free(&addr);
				break;
			case 5:
				printf("key: ");
				scanf("%s", k);
				val_a = (touchdb_val_t *)touchdb_list_pop(db, k);
				printf("[list : %s] => (%s)\n", k, touchdb_str_data(val_a));
				break;
			case 6:
				printf("key: ");
				scanf("%s", k);
				printf("index: ");
				scanf("%d", &idx);
				val_a = (touchdb_val_t *)touchdb_list_node_idx(db, k, idx);
				printf("[list : %s] => (index:%d) : (%s)\n", k, idx, touchdb_str_data(val_a));
				break;
			case 7:
				printf("key: ");
				scanf("%s", k);
				printf("[list : %s] size: %d\n", k, touchdb_list_size(db, k));
				break;
		}
	}
}
