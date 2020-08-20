#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    const char *name;
    int key;
} sysconf_var_t;

/* Variables from man sysconf */
const sysconf_var_t sysconf_vars[] = {
    /* POSIX.1 */
    { "ARG_MAX", _SC_ARG_MAX },
    { "CHILD_MAX", _SC_CHILD_MAX },
    { "HOST_NAME_MAX", _SC_HOST_NAME_MAX },
    { "LOGIN_NAME_MAX", _SC_LOGIN_NAME_MAX },
    { "NGROUPS_MAX", _SC_NGROUPS_MAX },
    { "OPEN_MAX", _SC_OPEN_MAX },
    { "PAGESIZE", _SC_PAGESIZE },
    { "RE_DUP_MAX", _SC_RE_DUP_MAX },
    { "STREAM_MAX", _SC_STREAM_MAX },
    { "SYMLOOP_MAX", _SC_SYMLOOP_MAX },
    { "TTY_NAME_MAX", _SC_TTY_NAME_MAX },
    { "TZNAME_MAX", _SC_TZNAME_MAX },
    { "_POSIX_VERSION", _SC_VERSION },

    /* POSIX.2 */
    { "BC_BASE_MAX", _SC_BC_BASE_MAX },
    { "BC_DIM_MAX", _SC_BC_DIM_MAX },
    { "BC_SCALE_MAX", _SC_BC_SCALE_MAX },
    { "COLL_WEIGHTS_MAX", _SC_COLL_WEIGHTS_MAX },
    { "EXPR_NEST_MAX", _SC_EXPR_NEST_MAX },
    { "LINE_MAX", _SC_LINE_MAX },
    { "RE_DUP_MAX", _SC_RE_DUP_MAX },
    { "POSIX2_VERSION", _SC_2_VERSION },
    { "POSIX2_C_DEV", _SC_2_C_DEV },
    { "POSIX2_FORT_DEV", _SC_2_FORT_DEV },
    { "POSIX2_FORT_RUN", _SC_2_FORT_RUN },
    { "_POSIX2_LOCALEDEF", _SC_2_LOCALEDEF },
    { "POSIX2_SW_DEV", _SC_2_SW_DEV },
};

void err(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    exit(1);
}

void print_sysconf(int val)
{
    long int res = sysconf(val);

    if (res == -1) {
        printf("undefined\n");
    } else {
        printf("%ld\n", res);
    }
}

int print_variable(const char *var)
{
    size_t items = sizeof(sysconf_vars) / sizeof(sysconf_var_t);
    const sysconf_var_t *item = sysconf_vars;
    const sysconf_var_t *max_item = item + items;

    if (!var)
        err("No variable");

    for (; item < max_item; item++) {
        if (strcmp(item->name, var) == 0) {
            print_sysconf(item->key);
            return 0;
        }
    }

    err("Unrecognized variable: '%s'\n", var);
    return 1;
}

int main(int argc, const char **argv)
{
    const char *varname;

    if (argc <= 1) {
        err("Usage: %s variable_name\n", argv[0]);
    }

    varname = argv[1];
    return print_variable(varname);
}
