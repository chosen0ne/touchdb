VPATH = ./src:./test
vpath %.h ./src
vpath %.c ./src
vpath %.c ./test

objects = touchdb_common.o touchdb_util.o touchdb_pool.o touchdb_map.o touchdb.o \
	touchdb_str.o touchdb_chain.o touchdb_worker.o
common = touchdb_common.c touchdb_common.h
util = touchdb_util.c touchdb_util.h
pool = touchdb_pool.c touchdb_pool.h
map = touchdb_map.c touchdb_map.h
str = touchdb_str.c touchdb_str.h
chain = touchdb_chain.c touchdb_chain.h
worker = touchdb_worker.c

map_tst: ${objects}
	gcc -g -lpthread -o map_tst ${objects}

touchdb.o : ${common} ${pool} ${chain} ${map} ${str}
	gcc -g -c src/touchdb.c

touchdb_common.o : ${common} ${str}
	gcc -g -c src/touchdb_common.c

touchdb_util.o : ${common} ${util}
	gcc -g -pthread -c src/touchdb_util.c

touchdb_pool.o : ${common} ${util} ${pool}
	gcc -g -c src/touchdb_pool.c

touchdb_map.o : ${common} ${util} ${pool} ${map}
	gcc -g -c src/touchdb_map.c

touchdb_str.o : ${common} ${pool} ${str}
	gcc -g -c src/touchdb_str.c

touchdb_chain.o : ${common} ${pool} ${str}
	gcc -g -c src/touchdb_chain.c

touchdb_worker.o : ${worker} touchdb.h
	gcc -g -c src/touchdb_worker.c

.PHONY : clean map_tst

clean:
	-rm map_tst ${objects}
