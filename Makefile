CFLAGS=-O3 -funroll-all-loops -march=nocona -mpopcnt -Wall -Wextra -D_GNU_SOURCE -I/tmp/gsl-1.9  -L/tmp/gsl-1.9/.libs -L/tmp/gsl-1.9/cblas/.libs
CFLAGS_DEBUG=-g -pg -fno-inline -static ${CFLAGS}
CFLAGS_RICE=-g -funit-at-a-time -fwhole-program -combine ${CFLAGS}
LDFLAGS=-lgsl -lgslcblas

UTIL_OBJECTS=util/linkedlist_u32.o util/linkedlist_u64.o util/linkedlist.o util/hashtable_u64.o util/hashmap_u64_int.o
UTIL_SOURCES=util/linkedlist_u32.c util/linkedlist_u64.c util/linkedlist.c util/hashtable_u64.c util/hashmap_u64_int.c

all: bistromath

rice: xboard.c engine.c book.c search.c transposition.c quiescent.c eval.c pawnstructure.c board.c movelist.c attacks.c popcnt.c rand.c ${UTIL_SOURCES}
	gcc ${CFLAGS_RICE} xboard.c engine.c book.c search.c transposition.c quiescent.c eval.c pawnstructure.c board.c movelist.c attacks.c popcnt.c rand.c ${UTIL_SOURCES} ${LDFLAGS} -o bistromath

debug: xboard.c engine.c book.c search.c transposition.c quiescent.c eval.c pawnstructure.c board.c movelist.c attacks.c popcnt.c rand.c ${UTIL_SOURCES}
	gcc ${CFLAGS_DEBUG} xboard.c engine.c book.c search.c transposition.c quiescent.c eval.c pawnstructure.c board.c movelist.c attacks.c popcnt.c rand.c ${UTIL_SOURCES} ${LDFLAGS} -o bistromath

bistromath: xboard.c engine book search transposition quiescent eval pawnstructure board movelist attacks popcnt rand ${UTIL_OBJECTS}
	gcc ${CFLAGS} xboard.c engine.o book.o search.o transposition.o quiescent.o eval.o pawnstructure.o board.o movelist.o attacks.o popcnt.o rand.o ${UTIL_OBJECTS} ${LDFLAGS} -o bistromath

engine: engine.c engine.h
	gcc ${CFLAGS} -c engine.c -o engine.o

book: book.c book.h
	gcc ${CFLAGS} -c book.c -o book.o

search: search.c search.h
	gcc ${CFLAGS} -c search.c -o search.o

transposition: transposition.c transposition.h
	gcc ${CFLAGS} -c transposition.c -o transposition.o

quiescent: quiescent.c quiescent.h
	gcc ${CFLAGS} -c quiescent.c -o quiescent.o

eval: eval.c eval.h
	gcc ${CFLAGS} -c eval.c -o eval.o

pawnstructure: pawnstructure.c pawnstructure.h
	gcc ${CFLAGS} -c pawnstructure.c -o pawnstructure.o

board: board.c board.h
	gcc ${CFLAGS} -c board.c -o board.o

movelist: movelist.c movelist.h
	gcc ${CFLAGS} -c movelist.c -o movelist.o

attacks: attacks.c attacks.h
	gcc ${CFLAGS} -c attacks.c -o attacks.o

popcnt: popcnt.c popcnt.h
	gcc ${CFLAGS} -c popcnt.c -o popcnt.o

rand: rand.c rand.h
	gcc ${CFLAGS} -c rand.c -o rand.o

clean:
	rm -f *.o bistromath
