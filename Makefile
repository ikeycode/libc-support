CFLAGS := -pedantic -Wall -Werror -O3

all: getconf

getconf: getconf.o

getconf.o: getconf.c

clean:
	rm -f *.o getconf
