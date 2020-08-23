CFLAGS := -pedantic -Wall -Werror -O3
SCAN_BUILD := scan-build

all: getconf

getconf: getconf.o

getconf.o: getconf.c

check: scan splint sparse

scan:
	@$(SCAN_BUILD) make

splint:
	@splint +posixlib *.c

sparse:
	@sparse *.c

clean:
	rm -f *.o getconf

.PHONY: all check scan clean
