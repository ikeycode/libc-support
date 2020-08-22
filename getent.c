#ifdef S_SPLINT_S
/* Hack for splint parse error caused by bits/thread-shared-types.h */
#define _THREAD_SHARED_TYPES_H
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <arpa/inet.h>

static const int addr_align_to = 16;
#define DST_LEN 256

enum {
    HELP_SHORT,
    HELP_FULL
};

enum {
    HOSTS_HOST,
    HOSTS_AHOST,
};

/* From bits/socket_type.h */
static const char *socktypes[] = {
    "",
    "STREAM ",     /* 1 */
    "DGRAM  ",     /* 2 */
    "RAW  ",       /* 3 */
    "RDM  ",       /* 4 */
    "SEQPACKET  ", /* 5 */
    "DCCP  ",      /* 6 */
    "",            /* 7 */
    "",            /* 8 */
    "",            /* 9 */
    "PACKET ",     /* 10 */
};
static const int socktype_size = sizeof(socktypes) / sizeof (const char *);

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

static void print_sockaddr(struct sockaddr *addr, int family, int align_to, int sock_type, int print_host)
{
    char dst[DST_LEN];
    char host[NI_MAXHOST];
    int cnt = 0;

    if (family == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *)addr;
        /*@-compdef@ @-mustfreefresh@ */
        cnt += printf("%s ", inet_ntop(AF_INET, (const void *)&sin->sin_addr, (char *)dst, DST_LEN));
        /*@=compdef@ @=mustfreefresh@*/
    } else {
        struct sockaddr_in6 *sin = (struct sockaddr_in6 *)addr;
        /*@-compdef@ @-mustfreefresh@*/
        cnt += printf("%s ", inet_ntop(AF_INET6, (const void *)&sin->sin6_addr, (char *)dst, DST_LEN));
        /*@=compdef@ @=mustfreefresh@*/
    }
    while (cnt++ < align_to)
        if (fputc(' ', stdout) == EOF)
            break;
    if (sock_type > 0 && sock_type < socktype_size)
        printf("%s", socktypes[sock_type]);
    if (print_host == 0) {
        printf("\n");
        return;
    }
    /*@-compdef@ @-type@ @-nullpass@ FIXME Missing alias names */
    (void)getnameinfo(addr, family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, 0);
    /*@=compdef@ @=type@ @=nullpass@*/
    /*@-type@*/
    host[NI_MAXHOST- 1] = 0;
    /*@=type@*/
    /*@-compdef@ @-nullpass@ @-usedef@*/
    printf("%s\n", host);
    /*@=compdef@ @=nullpass@ @=usedef@*/
}

static void print_host_info(const char *key, int host_type)
{
    struct addrinfo *info = NULL;
    int res = 0;

    /*@-nullpass@*/
    res = getaddrinfo(key, NULL, NULL, &info);
    /*@=nullpass@*/
    if (res != 0 || info == NULL)
        return;

    if (host_type == HOSTS_AHOST) {
        struct addrinfo *tmp = info;
        int print_host = 1;

        while (tmp != NULL) {
            print_sockaddr(tmp->ai_addr, tmp->ai_family, addr_align_to, tmp->ai_socktype, print_host);
            print_host = 0;
            tmp = tmp->ai_next;
        }
    } else
        print_sockaddr(info->ai_addr, info->ai_family, addr_align_to, 0, 1);

    freeaddrinfo(info);
}

static int get_hosts(/*@null@*/ const char **keys, int key_cnt, int host_type)
{
    if (keys == NULL)
        return 1;

    for (; key_cnt-- > 0; keys++)
        print_host_info(*keys, host_type);

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
    if (strcmp("ahosts", dbase) == 0) {
        if (keys != NULL)
            return get_hosts(keys, key_cnt, HOSTS_AHOST);
        return get_hosts_all();
    } else if (strcmp("hosts", dbase) == 0) {
        if (keys != NULL)
            return get_hosts(keys, key_cnt, HOSTS_HOST);
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
