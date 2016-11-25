/**
 * touchdb_str.c
 *
 * @author chosen0ne
 * @date 2012-06-13
 *
 * Copyright (C) by chosen0ne
 */

#include "touchdb_str.h"
#include <string.h>

touchdb_str_t	EMPTY_STR = {0, {-1, NIL}};

touchdb_val_t*
touchdb_str_new(touchdb_addr_t *addr, const char *str){
	touchdb_val_t 		*v;
	touchdb_str_t		*s;
	char				*data;

	v = touchdb_malloc(addr, sizeof(touchdb_str_t));
	v->type = TOUCHDB_VAL_STR;

	s = touchdb_malloc(&v->value, sizeof(touchdb_str_t));
	s->len = strlen(str);
	data = touchdb_malloc(&s->data, s->len + 1);
	strcpy(data, str);

	return v;
}

touchdb_str_t*
touchdb_str_new2(touchdb_val_t *val, const char *str){
	touchdb_str_t	*s;
	char			*data;

	val->type = TOUCHDB_VAL_STR;
	s = touchdb_malloc(&val->value, sizeof(touchdb_str_t));
	s->len = strlen(str) + 1;
	data = touchdb_malloc(&s->data, s->len);
	strcpy(data, str);

	return s;
}

int
touchdb_str_cmp(touchdb_val_t *v1, touchdb_val_t *v2){

	if( (v1->type == v2->type) && (v1->type == TOUCHDB_VAL_STR) ){
		// only compare when both v1 and v2 are TOUCHDB_VAL_STR

		touchdb_str_t *s1 = touchdb_ptr_of(&v1->value);
		touchdb_str_t *s2 = touchdb_ptr_of(&v2->value);

		const char *raw_s1 = touchdb_ptr_of(&s1->data);
		const char *raw_s2 = touchdb_ptr_of(&s2->data);

		return strcmp(raw_s1, raw_s2);
	}else{
		return -1;
	}
}

const char*
touchdb_str_data(touchdb_val_t *s){
	if(s == NULL)
		return NULL;

	if(s->type == TOUCHDB_VAL_STR){
		touchdb_str_t	*str = (touchdb_str_t *)touchdb_ptr_of(&s->value);

		return (char *)touchdb_ptr_of(&str->data);
	}

	return NULL;
}

int
touchdb_str_cpy(touchdb_val_t *dest, touchdb_val_t *src, int n){
	if( (dest->type == src->type) && (dest->type == TOUCHDB_VAL_STR) ){
		char			*dest_s, *src_s;
		touchdb_str_t 	*dest_str = touchdb_ptr_of(&dest->value);
		touchdb_str_t 	*src_str = touchdb_ptr_of(&src->value);

		touchdb_free(&dest_str->data);
		src_s = touchdb_ptr_of(&src_str->data);
		if( (dest_s = touchdb_malloc(&dest_str->data, strlen(src_s) + 1)) == NULL){
			touchdb_log(TOUCHDB_LOG_ERR, "[touchdb_str_cpy]touchdb_malloc() error! NO enough space");
			return -1;
		}
		strcpy(dest_s, src_s);

		return 0;
	}else{
		return -1;
	}
}

void
touchdb_str_destory(touchdb_val_t *str){
	touchdb_str_t	*s = touchdb_ptr_of(&str->value);

	touchdb_free(&s->data);
	touchdb_free(&str->value);
}
