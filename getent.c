#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

enum {
    HELP_SHORT,
    HELP_FULL
};

static void err(const char *msg, ...)
{
    va_list args;

    /*@-nullpass@*/
    va_start(args, msg);
    (void)vfprintf(stderr, msg, args);
    va_end(args);
    /*@=nullpass@*/

    exit(EXIT_FAILURE);
}

static void usage(const char *name, int help)
{
    fprintf(stderr, "Usage: %s database [key]\n", name);

    if (help == HELP_FULL) {
        fprintf(stderr, "\nArguments:\n");
        fprintf(stderr, "  database     Database to query from\n");
        fprintf(stderr, "  key          Key to query\n");
    }

    exit(EXIT_FAILURE);
}

static int read_hosts(/*@null@*/ const char *key)
{
    if (key == NULL)
        return 1;

    return 0;
}

static int read_database(const char *dbase, /*@null@*/ const char *key)
{
    if (strcmp("hosts", dbase) == 0)
        return read_hosts(key);

    err("Unknown database: %s\n", dbase);
    return 1;
}

int main(int argc, char **argv)
{
    const char *dbase = NULL;
    const char *key = NULL;
    int index = 1;

    for (index = 1; index < argc; index++) {
        if (strcmp("-h", argv[index]) == 0)
            usage(argv[0], HELP_FULL);
        else if (dbase == NULL)
            dbase = argv[index];
        else if (key == NULL)
            key = argv[index];
        else
            err("Unknown parameter: %s\n", argv[index]);
    }

    if (dbase == NULL)
        usage(argv[0], HELP_SHORT);

    /*@-nullpass@*/
    return read_database(dbase, key);
    /*@=nullpass@*/
}
