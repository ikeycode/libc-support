CFLAGS = -pedantic -Wall -Werror

all: getconf

getconf: getconf.o

getconf.o: getconf.c

clean:
	rm -f *.o getconf
