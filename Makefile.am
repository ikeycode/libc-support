ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = ${top_srcdir}/README.md \
	     ${top_srcdir}/LICENSE   \
	     ${top_srcdir}/src/getconf/getconf.inc
	     ${top_srcdir}/src/getent/getent.inc

NULL =
CLEANFILES =

AM_CFLAGS = -fstack-protector -Wall -pedantic \
        -Wstrict-prototypes -Wundef -fno-common \
        -Werror-implicit-function-declaration \
        -Wformat -Wformat-security -Werror=format-security \
        -Wconversion -Wunused-variable -Wunreachable-code \
        -Wall -W -D_FORTIFY_SOURCE=2 \
        -std=c11

AM_CPPFLAGS = 					 \
	-I $(top_srcdir)/src \
	-I $(top_srcdir)/src/getconf \
	-I $(top_srcdir)/src/getent

bin_PROGRAMS = \
	getconf \
	getent

getconf_SOURCES =			\
	src/getconf/getconf.c

getconf_CFLAGS =	\
	$(AM_CFLAGS)

getent_SOURCES =			\
	src/getent/db_hosts.c		\
	src/getent/db_networks.c	\
	src/getent/db_password.c	\
	src/getent/db_aliases.c		\
	src/getent/db_ethers.c		\
	src/getent/db_protocols.c	\
	src/getent/db_group.c		\
	src/getent/db_shadow.c		\
	src/getent/db_gshadow.c		\
	src/getent/db_services.c	\
	src/getent/db_rpc.c		\
	src/getent/db_initgroups.c	\
	src/getent/db_netgroup.c	\
	src/getent/getent.h		\
	src/getent/getent.c

getent_CFLAGS =		\
	$(AM_CFLAGS)
