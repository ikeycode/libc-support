#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

static const int addr_align_to = 16;

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

static void print_addr(char *addr, int len, unsigned int align_to)
{
    int first = 1;
    int cnt = 0;

    while (len-- > 0) {
        cnt += printf("%s%hhu", first == 0 ? "." : "", *addr++);
        first = 0;
    }
    while (cnt++ < align_to)
        if (fputc(' ', stdout) == EOF)
            break;
}

static int read_hosts(/*@null@*/ const char *key)
{
    struct hostent *ent = NULL;

    sethostent(0);
    while ((ent = gethostent()) != NULL) {
        print_addr(ent->h_addr_list[0], ent->h_length, addr_align_to);
        printf("%s\n", ent->h_name);
    }
    endhostent();

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
