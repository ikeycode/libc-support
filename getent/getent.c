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
#include <aliases.h>
#include <netinet/ether.h>
#include <shadow.h>
#include <grp.h>
#include <pwd.h>
#include <gshadow.h>

static const int addr_align_to = 16;
static const int alias_align_to = 16;
#define DST_LEN 256

enum {
    HELP_SHORT,
    HELP_FULL
};

enum {
    HOSTS_HOST,
    HOSTS_AHOST,
    HOSTS_AHOST_V4,
    HOSTS_AHOST_V6,
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

static const size_t socktype_size = sizeof(socktypes) / sizeof (const char *);

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

static void print_align_to(int cnt, int align_to)
{
    while (++cnt < align_to)
        if (fputc(' ', stdout) == EOF)
            break;
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
    print_align_to(cnt, align_to);
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
    print_align_to(cnt, align_to);
    if (sock_type > 0 && (size_t)sock_type < socktype_size)
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
    struct addrinfo hints;
    int res = 0;

    memset(&hints, 0, sizeof(struct addrinfo));
    if (host_type == HOSTS_AHOST_V6) {
        hints.ai_family = AF_INET6;
        hints.ai_flags = AI_V4MAPPED;
    } else if (host_type == HOSTS_AHOST_V4) {
        hints.ai_family = AF_INET;
    }
    /*@-nullpass@*/
    res = getaddrinfo(key, NULL, &hints, &info);
    /*@=nullpass@*/
    if (res != 0 || info == NULL)
    /*@-compdestroy@*/
        return;
    /*@=compdestroy@*/

    if (host_type == HOSTS_AHOST || host_type == HOSTS_AHOST_V4 || host_type == HOSTS_AHOST_V6) {
        struct addrinfo *tmp = info;
        int print_host = 1;

        for (; tmp != NULL; tmp = tmp->ai_next) {
            print_sockaddr(tmp->ai_addr, tmp->ai_family, addr_align_to, tmp->ai_socktype, print_host);
            print_host = 0;
        }
    } else
        print_sockaddr(info->ai_addr, info->ai_family, addr_align_to, 0, 1);

    freeaddrinfo(info);
    /*@-compdestroy@*/
}
/*@=compdestroy@*/

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

static void print_alias_info(struct aliasent *ent)
{
    size_t i;
    int cnt = 0;

    cnt += printf("%s: ", ent->alias_name);
    print_align_to(cnt, alias_align_to);
    for (i = 0; i < ent->alias_members_len; i++) {
            printf(" %s", ent->alias_members[i]);
    }
    printf("\n");
}

static int get_aliases(/*@null@*/ const char **keys, int key_cnt)
{
    if (keys == NULL)
        return 1;

    for (; key_cnt-- > 0; keys++) {
        struct aliasent *ent = NULL;

        /*@-mustfreefresh@*/
        ent = getaliasbyname(*keys);
        if (ent != NULL)
            print_alias_info(ent);
    }
    /*@=mustfreefresh@*/

    return 0;
}

static int get_aliases_all(void)
{
    struct aliasent *ent = NULL;

    setaliasent();
    while ((ent = getaliasent()) != NULL)
        print_alias_info(ent);
    endaliasent();

    return 0;
}

static int get_ethers(/*@null@*/ const char **keys, int key_cnt)
{
    struct ether_addr *addr = NULL;
    struct ether_addr addr_dst;
    if (keys == NULL)
        return 1;

    for (; key_cnt-- > 0; keys++) {
        char hostname[NI_MAXHOST];
        int res = 0;

        addr = ether_aton(*keys);
        if (addr != NULL) {
            char *addr_str = ether_ntoa(addr);
            /*@-mustfreefresh@*/
            if (addr_str == NULL)
                return 2;
            printf("%s ", addr_str);
        } else {
            /*@=mustfreefresh@*/
            memset(&addr_dst, 0, sizeof(struct ether_addr));
            res = ether_hostton(*keys, &addr_dst);

            if (res != 0)
                return 2;
            /*@-mustfreefresh@*/
            printf("%s ", ether_ntoa(&addr_dst));
            /*@=mustfreefresh@*/
            /*@-branchstate@*/
            addr = &addr_dst;
        }
        /*@=branchstate@*/
        /*@-compdef@*/
        res = ether_ntohost(hostname, addr);
        if (res != 0)
            return 2;
        /*@-usedef@*/
        printf("%s\n", hostname);
        /*@=compdef@ @=usedef@*/
    }

    return 0;
}

static void print_shadow_info(struct spwd *pwd)
{
    printf("%s", pwd->sp_namp);
    printf(":%s", pwd->sp_pwdp);
    printf(":%ld", pwd->sp_lstchg);
    printf(":");
    if (pwd->sp_min >= 0)
        printf("%ld", pwd->sp_min);
    printf(":");
    if (pwd->sp_max >= 0)
        printf("%ld", pwd->sp_max);
    printf(":");
    if (pwd->sp_warn >= 0)
        printf("%ld", pwd->sp_warn);
    printf(":");
    if (pwd->sp_inact >= 0)
        printf(":%ld", pwd->sp_inact);
    printf(":");
    if (pwd->sp_expire >= 0)
        printf(":%ld", pwd->sp_expire);
    printf(":");
    if (pwd->sp_flag != (unsigned long)-1)
        printf(":%lu", pwd->sp_flag);
    printf("\n");
}

static int get_shadow(/*@null@*/ const char **keys, int key_cnt)
{
    if (keys == NULL)
        return 1;

    for (; key_cnt-- > 0; keys++) {
        struct spwd *pwd = NULL;

        /*@-mustfreefresh@*/
        pwd = getspnam(*keys);
        if (pwd != NULL)
            print_shadow_info(pwd);
    }
    /*@=mustfreefresh@*/

    return 0;
}

static int get_shadow_all(void)
{
    struct spwd *pwd = NULL;

    setspent();
    while ((pwd = getspent()) != NULL)
        print_shadow_info(pwd);
    endspent();

    return 0;
}

static void print_gshadow_info(struct sgrp *pwd)
{
    char **memb = NULL;
    int first = 1;

    printf("%s", pwd->sg_namp);
    printf(":%s", pwd->sg_passwd);
    printf(":");
    for (first = 1, memb = pwd->sg_adm; *memb != NULL; memb++) {
        printf("%s%s", first == 0 ? "," : "", *memb);
        first = 0;
    }
    printf(":");
    for (first = 1, memb = pwd->sg_mem; *memb != NULL; memb++) {
        printf("%s%s", first == 0 ? "," : "", *memb);
        first = 0;
    }
    printf("\n");
}

static int get_gshadow(/*@null@*/ const char **keys, int key_cnt)
{
    if (keys == NULL)
        return 1;

    for (; key_cnt-- > 0; keys++) {
        struct sgrp *pwd = NULL;

        /*@-mustfreefresh@*/
        pwd = getsgnam(*keys);
        if (pwd != NULL)
            print_gshadow_info(pwd);
    }
    /*@=mustfreefresh@*/

    return 0;
}

static int get_gshadow_all(void)
{
    struct sgrp *pwd = NULL;

    setsgent();
    while ((pwd = getsgent()) != NULL)
        print_gshadow_info(pwd);
    endsgent();

    return 0;
}

static void print_passwd_info(struct passwd *pwd)
{
    printf("%s", pwd->pw_name);
    printf(":%s", pwd->pw_passwd);
    printf(":%u", pwd->pw_uid);
    printf(":%u", pwd->pw_gid);
    printf(":");
    if (pwd->pw_gecos != NULL)
        printf("%s", pwd->pw_gecos);
    printf(":");
    if (pwd->pw_dir != NULL)
        printf("%s", pwd->pw_dir);
    printf(":");
    if (pwd->pw_shell != NULL)
        printf("%s", pwd->pw_shell);
    printf("\n");
}

static int get_passwd(/*@null@*/ const char **keys, int key_cnt)
{
    if (keys == NULL)
        return 1;

    for (; key_cnt-- > 0; keys++) {
        struct passwd *pwd = NULL;

        /*@-mustfreefresh@*/
        pwd = getpwnam(*keys);
        if (pwd != NULL)
            print_passwd_info(pwd);
    }
    /*@=mustfreefresh@*/

    return 0;
}

static int get_passwd_all(void)
{
    struct passwd *pwd = NULL;

    setpwent();
    while ((pwd = getpwent()) != NULL)
        print_passwd_info(pwd);
    endpwent();

    return 0;
}

static void print_group_info(struct group *grp)
{
    char **memb = NULL;
    int first = 1;

    printf("%s", grp->gr_name);
    printf(":%s", grp->gr_passwd);
    printf(":");
    printf("%u", grp->gr_gid);
    printf(":");
    for (memb = grp->gr_mem; *memb != NULL; memb++) {
        printf("%s%s", first == 0 ? "," : "", *memb);
        first = 0;
    }
    printf("\n");
}

static int is_numeric(const char *v)
{
    for (; v != NULL && *v != (char)0; v++)
        if (*v < '0' || *v > '9')
            return 0;

    return 1;
}

static int get_group(/*@null@*/ const char **keys, int key_cnt)
{
    if (keys == NULL)
        return 1;

    for (; key_cnt-- > 0; keys++) {
        struct group *grp = NULL;

        /*@-mustfreefresh@*/
        if (is_numeric(*keys) == 1)
            /*@-type@*/
            grp = getgrgid(atoi(*keys));
            /*@=type@*/
        else
            grp = getgrnam(*keys);
        if (grp != NULL)
            print_group_info(grp);
    }
    /*@=mustfreefresh@*/

    return 0;
}

static int get_group_all(void)
{
    struct group *grp = NULL;

    setgrent();
    while ((grp = getgrent()) != NULL)
        print_group_info(grp);
    endgrent();

    return 0;
}

static int read_database(const char *dbase, /*@null@*/ const char **keys, int key_cnt)
{
    /*
     * Missing:
     *  initgroups
     *  netgroup
     *  networks
     *  protocols
     *  rpc
     *  services
     */
    if (strcmp("ahosts", dbase) == 0) {
        if (keys != NULL)
            return get_hosts(keys, key_cnt, HOSTS_AHOST);
        return get_hosts_all();
    } else if (strcmp("ahostsv4", dbase) == 0) {
        if (keys != NULL)
            return get_hosts(keys, key_cnt, HOSTS_AHOST_V4);
        return get_hosts_all();
    } else if (strcmp("ahostsv6", dbase) == 0) {
        if (keys != NULL)
            return get_hosts(keys, key_cnt, HOSTS_AHOST_V6);
        return get_hosts_all();
    } else if (strcmp("hosts", dbase) == 0) {
        if (keys != NULL)
            return get_hosts(keys, key_cnt, HOSTS_HOST);
        return get_hosts_all();
    } else if (strcmp("aliases", dbase) == 0) {
        if (keys != NULL)
            return get_aliases(keys, key_cnt);
        return get_aliases_all();
    } else if (strcmp("ethers", dbase) == 0) {
        if (keys != NULL)
            return get_ethers(keys, key_cnt);
        printf("Enumeration not supported on ethers\n");
        return 3;
    } else if (strcmp("group", dbase) == 0) {
        if (keys != NULL)
            return get_group(keys, key_cnt);
        return get_group_all();
    } else if (strcmp("gshadow", dbase) == 0) {
        if (keys != NULL)
            return get_gshadow(keys, key_cnt);
        return get_gshadow_all();
    } else if (strcmp("passwd", dbase) == 0) {
        if (keys != NULL)
            return get_passwd(keys, key_cnt);
        return get_passwd_all();
    } else if (strcmp("shadow", dbase) == 0) {
        if (keys != NULL)
            return get_shadow(keys, key_cnt);
        return get_shadow_all();
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
        else if (strcmp("-s", argv[index]) == 0 || strcmp("--service", argv[index]) == 0) {
            index++;
            /* Ignored for now */
        } else if (strcmp("-i", argv[index]) == 0) {
            /* Ignored for now */
        } else if (dbase == NULL)
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
