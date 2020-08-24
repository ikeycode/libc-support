CFLAGS := -pedantic -Wall -Werror -O3
SCAN_BUILD := scan-build

all: getent

getent: getent.o

getent.o: getent.c

check: scan sparse

scan:
	@$(SCAN_BUILD) make

splint:
	@splint +posixlib -D__ONCE_ALIGNMENT= -D__signed__= *.c

sparse:
	@sparse *.c

clean:
	rm -f *.o getent

.PHONY: all check scan clean
