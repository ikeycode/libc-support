#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#ifdef S_SPLINT_S
/* Hack for splint parse error caused by bits/thread-shared-types.h */
#define _THREAD_SHARED_TYPES_H
#endif
#include <netdb.h>
#include <arpa/inet.h>

static const int addr_align_to = 16;
#define DST_LEN 256
#define HOST_LEN 256

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

static void print_addr(char *addr, int len, int align_to)
{
    int first = 1;
    int cnt = 0;

    while (len-- > 0) {
        /*@+charint@*/
        cnt += printf("%s%hhu", first == 0 ? "." : "", (unsigned char)*addr);
        /*@-charint@*/
        addr++;
        first = 0;
    }
    printf(" ");
    while (++cnt < align_to)
        if (fputc(' ', stdout) == EOF)
            break;
}

static void print_sockaddr(struct sockaddr *addr, int family)
{
    char dst[DST_LEN];
    char host[HOST_LEN];

    if (family == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *)addr;
        printf("%s ", inet_ntop(AF_INET, (const void *)&sin->sin_addr, (char *)dst, DST_LEN));
    } else {
        struct sockaddr_in6 *sin = (struct sockaddr_in6 *)addr;
        printf("%s ", inet_ntop(AF_INET6, (const void *)&sin->sin6_addr, (char *)dst, DST_LEN));
    }
    (void)getnameinfo(addr, family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, HOST_LEN, NULL, 0, 0);
    host[HOST_LEN - 1] = 0;
    printf("%s\n", host);
}

static void print_by_host(const char *key)
{
    struct addrinfo *info = NULL;
    int res = 0;

    res = getaddrinfo(key, NULL, NULL, &info);
    if (res != 0)
        return;

    print_sockaddr(info->ai_addr, info->ai_family);

    freeaddrinfo(info);
}

static int get_hosts(/*@null@*/ const char **keys, int key_cnt)
{
    if (keys == NULL)
        return 1;

    for (; key_cnt-- > 0; keys++) {
        char dst6[sizeof(struct in6_addr)];
        char dst[sizeof(struct in6_addr)];
        /*@-compdef@*/
        int addr6 = inet_pton(AF_INET6, *keys, dst6);
        int addr = inet_pton(AF_INET, *keys, dst);
        /*@=compdef@*/

        if (addr == -1 && addr6 == -1)
            continue;
        else if (addr == 0 && addr6 == 0) {
            print_by_host(*keys);
        } else if (addr6 == 1) {
        } else
            ;

    }

    return 0;
}

static int get_hosts_all(void)
{
    struct hostent *ent = NULL;

    sethostent(0);
    while ((ent = gethostent()) != NULL) {
        char **aliases = ent->h_aliases;

        print_addr(ent->h_addr_list[0], ent->h_length, addr_align_to);
        printf("%s", ent->h_name);
        while (aliases != NULL && *aliases != NULL) {
            printf(" %s", *aliases);
            aliases++;
        }
        printf("\n");
    }
    endhostent();

    return 0;
}

static int read_database(const char *dbase, /*@null@*/ const char **keys, int key_cnt)
{
    if (strcmp("hosts", dbase) == 0) {
        if (keys != NULL)
            return get_hosts(keys, key_cnt);
        return get_hosts_all();
    }

    err("Unknown database: %s\n", dbase);
    return 1;
}

int main(int argc, char **argv)
{
    const char *dbase = NULL;
    const char **keys = NULL;
    int index = 1;

    for (index = 1; index < argc; index++) {
        if (strcmp("-h", argv[index]) == 0)
            usage(argv[0], HELP_FULL);
        else if (dbase == NULL)
            dbase = argv[index];
        else if (keys == NULL) {
            keys = (const char **)&argv[index];
            break;
        }
        else
            err("Unknown parameter: %s\n", argv[index]);
    }

    if (dbase == NULL)
        usage(argv[0], HELP_SHORT);

    /*@-nullpass@*/
    return read_database(dbase, keys, argc - index);
    /*@=nullpass@*/
}
