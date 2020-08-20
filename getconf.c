#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    const char *name;
    int key;
} sysconf_var_t;

const sysconf_var_t sysconf_vars[] = {
    { "ARG_MAX", _SC_ARG_MAX },
};

void err(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    exit(1);
}

int print_variable(const char *var)
{
    const sysconf_var_t *item = sysconf_vars;

    if (strcmp(item->name, var) == 0) {
        printf("%ld\n", sysconf(item->key));
        return 0;
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
