#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

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
