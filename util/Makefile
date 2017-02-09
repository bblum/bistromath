CFLAGS=-O3 -march=nocona -Wall -Wextra

all: hashmap_u64_u64.o hashmap_u64_int.o hashtable.o hashtable_u64.o linkedlist.o linkedlist_u64.o linkedlist_u32.o

hashmap_u64_u64: hashmap_u64_u64.c hashmap_u64_u64.h
	gcc ${CFLAGS} -c hashmap_u64_u64.c

hashmap_u64_int: hashmap_u64_int.c hashmap_u64_int.h
	gcc ${CFLAGS} -c hashmap_u64_int.c

hashmap_int_int: hashmap_int_int.c hashmap_int_int.h
	gcc ${CFLAGS} -c hashmap_int_int.c

hashtable: hashtable.c hashtable.h
	gcc ${CFLAGS} -c hashtable.c

hashtable_u64: hashtable_u64.c hashtable_u64.h
	gcc ${CFLAGS} -c hashtable_u64.c

linkedlist: linkedlist.c linkedlist.h
	gcc ${CFLAGS} -c linkedlist.c

linkedlist_u64: linkedlist_u64.c linkedlist_u64.h
	gcc ${CFLAGS} -c linkedlist_u64.c

linkedlist_u32: linkedlist_u32.c linkedlist_u32.h
	gcc ${CFLAGS} -c linkedlist_u32.c

clean:
	rm -f *.o
