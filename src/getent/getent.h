#ifndef GETENT_H
#define GETENT_H

#include <stdio.h>

#include "config.h"

#ifndef HAVE_ALIASES
#define HAVE_ALIASES 0
#endif
#ifndef HAVE_GSHADOW
#define HAVE_GSHADOW 0
#endif
#ifndef HAVE_RPC
#define HAVE_RPC 0
#endif
#ifndef HAVE_NETGROUP
#define HAVE_NETGROUP 0
#endif

enum { RES_OK = 0,
       RES_MISSING_ARG_OR_INVALID_DATABASE = 1,
       RES_KEY_NOT_FOUND = 2,
       RES_ENUMERATION_NOT_SUPPORTED = 3,
};

typedef int (*get_func_t)(const char **keys, int key_cnt);
typedef int (*enum_func_t)(void);

typedef struct getconf_database_config {
        const char *name;
        get_func_t get;
        enum_func_t enum_all;
} getconf_database_config_t;

#define DST_LEN 256

#define EXT_DB(X)                                                                                  \
        extern int get_##X(const char **keys, int key_cnt);                                        \
        extern int enum_##X##_all(void);

#define DATABASE_CONF(X)                                                                           \
        {                                                                                          \
                .name = #X, .get = get_##X, .enum_all = enum_##X##_all                             \
        }
#define DATABASE_CONF_HOSTS(X)                                                                     \
        {                                                                                          \
                .name = #X, .get = get_##X, .enum_all = enum_hosts_all                             \
        }

#define ENUM_ALL(X, base, initparm, type)                                                          \
        int enum_##X##_all(void)                                                                   \
        {                                                                                          \
                struct type *ent = NULL;                                                           \
                set##base(initparm);                                                               \
                while ((ent = get##base()) != NULL)                                                \
                        print_##type##_info(ent);                                                  \
                end##base();                                                                       \
                return RES_OK;                                                                     \
        }

#define GET_SIMPLE(X, getfunc, type)                                                               \
        int get_##X(const char **keys, int key_cnt)                                                \
        {                                                                                          \
                if (keys == NULL)                                                                  \
                        return RES_KEY_NOT_FOUND;                                                  \
                for (; key_cnt-- > 0; keys++) {                                                    \
                        struct type *ent = NULL;                                                   \
                        ent = getfunc(*keys);                                                      \
                        if (ent != NULL)                                                           \
                                print_##type##_info(ent);                                          \
                }                                                                                  \
                return RES_OK;                                                                     \
        }

#define GET_NUMERIC_CAST(X, getfunc, getnumericfunc, type, numcast)                                \
        int get_##X(const char **keys, int key_cnt)                                                \
        {                                                                                          \
                if (keys == NULL)                                                                  \
                        return RES_KEY_NOT_FOUND;                                                  \
                for (; key_cnt-- > 0; keys++) {                                                    \
                        struct type *ent = NULL;                                                   \
                        if (*keys == NULL)                                                         \
                                continue;                                                          \
                        if (is_numeric(*keys) == 1)                                                \
                                ent = getnumericfunc((numcast)atoi(*keys));                        \
                        else                                                                       \
                                ent = getfunc(*keys);                                              \
                        if (ent != NULL)                                                           \
                                print_##type##_info(ent);                                          \
                }                                                                                  \
                return RES_OK;                                                                     \
        }
#define GET_NUMERIC(X, getfunc, getnumericfunc, type)                                              \
        GET_NUMERIC_CAST(X, getfunc, getnumericfunc, type, int)

#define NO_ENUM_ALL_FOR(X)                                                                         \
        int enum_##X##_all(void)                                                                   \
        {                                                                                          \
                return no_enum(#X);                                                                \
        }

static inline void print_align_to(int cnt, int align_to)
{
        while (++cnt < align_to)
                if (fputc(' ', stdout) == EOF)
                        break;
}

static inline int no_enum(const char *db)
{
        printf("Enumeration not supported on %s\n", db);
        return RES_ENUMERATION_NOT_SUPPORTED;
}

int is_numeric(const char *v);
void err(const char *msg, ...);

#endif
