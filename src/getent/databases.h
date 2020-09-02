#ifndef __DATABASES_H
#define __DATABASES_H

#include "getent.h"

#define DB(X)                                                                                      \
        extern int get_##X(const char **keys, int key_cnt);                                        \
        extern int enum_##X##_all(void);
#include "getent.inc"
#undef DB

#define DB(X) DATABASE_CONF(X),
static const getconf_database_config_t databases[] = {
#include "getent.inc"
};
#undef DB
static const size_t databases_size = sizeof(databases) / sizeof(getconf_database_config_t);

#endif
