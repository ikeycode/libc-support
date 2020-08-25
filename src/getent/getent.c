/*
 * This file is part of libc-support.
 *
 * Copyright © 2020 Serpent OS Developers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <getopt.h>
#include <grp.h>
#include <locale.h>
#include <netdb.h>
#include <netinet/ether.h>
#include <pwd.h>
#include <shadow.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

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

#if HAVE_ALIASES
#include <aliases.h>
#endif
#if HAVE_GSHADOW
#include <gshadow.h>
#endif

static const int addr_align_to = 16;
#if HAVE_ALIASES
static const int alias_align_to = 16;
#endif
static const int network_align_to = 23;
#if HAVE_RPC
static const int rpc_align_to = 17;
#endif
static const int proto_align_to = 23;
static const int initgroup_align_to = 23;
#if HAVE_NETGROUP
static const int netgroup_align_to = 23;
#endif

#define DST_LEN 256
static const size_t MAX_GROUP_CNT = 256;

enum { HELP_SHORT, HELP_FULL };

enum { HOSTS_HOST,
       HOSTS_AHOST,
       HOSTS_AHOST_V4,
       HOSTS_AHOST_V6,
};

enum { RES_OK = 0,
       RES_MISSING_ARG_OR_INVALID_DATABASE = 1,
       RES_KEY_NOT_FOUND = 2,
       RES_ENUMERATION_NOT_SUPPORTED = 3,
};

/* From bits/socket_type.h */
static const char *socktypes[] = {
        [SOCK_STREAM] = "STREAM", [SOCK_DGRAM] = "DGRAM",         [SOCK_RAW] = "RAW",
        [SOCK_RDM] = "RDM",       [SOCK_SEQPACKET] = "SEQPACKET", [SOCK_DCCP] = "DCCP",
        [SOCK_PACKET] = "PACKET"
};

static const size_t socktype_size = sizeof(socktypes) / sizeof(const char *);

typedef int (*get_func_t)(const char **keys, int key_cnt);
typedef int (*enum_func_t)(void);

typedef struct getconf_database_config {
        const char *name;
        get_func_t get;
        enum_func_t enum_all;
} getconf_database_config_t;

#define DATABASE_CONF(X)                                                                           \
        {                                                                                          \
#X, get_##X, enum_##X##_all                                                        \
        }
#define DATABASE_CONF_HOSTS(X)                                                                     \
        {                                                                                          \
#X, get_##X, enum_hosts_all                                                        \
        }

#define ENUM_ALL(X, base, initparm, type)                                                          \
        static int enum_##X##_all(void)                                                            \
        {                                                                                          \
                struct type *ent = NULL;                                                           \
                set##base(initparm);                                                               \
                while ((ent = get##base()) != NULL)                                                \
                        print_##type##_info(ent);                                                  \
                end##base();                                                                       \
                return RES_OK;                                                                     \
        }

#define GET_SIMPLE(X, getfunc, type)                                                               \
        static int get_##X(const char **keys, int key_cnt)                                         \
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
        static int get_##X(const char **keys, int key_cnt)                                         \
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
        static int enum_##X##_all(void)                                                            \
        {                                                                                          \
                return no_enum(#X);                                                                \
        }

static void err(const char *msg, ...)
{
        va_list args;

        va_start(args, msg);
        (void)vfprintf(stderr, msg, args);
        va_end(args);

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
                cnt += printf("%s%hhu", first == 0 ? "." : "", (unsigned char)*addr);
                addr++;
                first = 0;
        }
        printf(" ");
        print_align_to(cnt, align_to);
}

static void print_sockaddr(struct sockaddr *addr, int family, int align_to, int sock_type,
                           int print_host)
{
        char dst[DST_LEN];
        char host[NI_MAXHOST];
        int cnt = 0;

        if (family == AF_INET) {
                struct sockaddr_in *sin = (struct sockaddr_in *)addr;
                cnt +=
                    printf("%s ",
                           inet_ntop(AF_INET, (const void *)&sin->sin_addr, (char *)dst, DST_LEN));
        } else {
                struct sockaddr_in6 *sin = (struct sockaddr_in6 *)addr;
                cnt += printf("%s ",
                              inet_ntop(AF_INET6,
                                        (const void *)&sin->sin6_addr,
                                        (char *)dst,
                                        DST_LEN));
        }
        print_align_to(cnt, align_to);
        if (sock_type > 0 && (size_t)sock_type < socktype_size)
                printf("%s", socktypes[sock_type]);
        if (print_host == 0) {
                printf("\n");
                return;
        }
        (void)getnameinfo(addr,
                          family == AF_INET ? sizeof(struct sockaddr_in)
                                            : sizeof(struct sockaddr_in6),
                          host,
                          NI_MAXHOST,
                          NULL,
                          0,
                          0);
        host[NI_MAXHOST - 1] = 0;
        printf(" %s\n", host);
}

static void print_single_host_info(const char *key, int host_type)
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
        res = getaddrinfo(key, NULL, &hints, &info);
        if (res != 0 || info == NULL)
                return;

        if (host_type == HOSTS_AHOST || host_type == HOSTS_AHOST_V4 ||
            host_type == HOSTS_AHOST_V6) {
                struct addrinfo *tmp = NULL;
                int print_host = 1;

                for (tmp = info; tmp != NULL; tmp = tmp->ai_next) {
                        print_sockaddr(tmp->ai_addr,
                                       tmp->ai_family,
                                       addr_align_to,
                                       tmp->ai_socktype,
                                       print_host);
                        print_host = 0;
                }
        } else
                print_sockaddr(info->ai_addr, info->ai_family, addr_align_to, 0, 1);

        freeaddrinfo(info);
}

static int _get_hosts(const char **keys, int key_cnt, int host_type)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++)
                print_single_host_info(*keys, host_type);

        return RES_OK;
}

static void print_hostent_info(struct hostent *ent)
{
        char **aliases = ent->h_aliases;

        print_addr(ent->h_addr_list[0], ent->h_length, addr_align_to);
        printf("%s", ent->h_name);
        while (aliases != NULL && *aliases != NULL) {
                printf(" %s", *aliases);
                aliases++;
        }
        printf("\n");
}

#if HAVE_ALIASES
static void print_aliasent_info(struct aliasent *ent)
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
#endif

static int get_ethers(const char **keys, int key_cnt)
{
        struct ether_addr *addr = NULL;
        struct ether_addr addr_dst;
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++) {
                char hostname[NI_MAXHOST];
                int res = 0;

                addr = ether_aton(*keys);
                if (addr != NULL) {
                        char *addr_str = ether_ntoa(addr);
                        if (addr_str == NULL)
                                return RES_KEY_NOT_FOUND;
                        printf("%s ", addr_str);
                } else {
                        memset(&addr_dst, 0, sizeof(struct ether_addr));
                        res = ether_hostton(*keys, &addr_dst);

                        if (res != 0)
                                return RES_KEY_NOT_FOUND;
                        printf("%s ", ether_ntoa(&addr_dst));
                        addr = &addr_dst;
                }
                res = ether_ntohost(hostname, addr);
                if (res != 0)
                        return RES_KEY_NOT_FOUND;
                printf("%s\n", hostname);
        }

        return RES_OK;
}

static void print_spwd_info(struct spwd *pwd)
{
        printf("%s:%s:%ld:", pwd->sp_namp, pwd->sp_pwdp, pwd->sp_lstchg);
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
                printf("%ld", pwd->sp_inact);
        printf(":");
        if (pwd->sp_expire >= 0)
                printf("%ld", pwd->sp_expire);
        printf(":");
        if (pwd->sp_flag != (unsigned long)-1)
                printf("%lu", pwd->sp_flag);
        printf("\n");
}

#if HAVE_GSHADOW
static void print_sgrp_info(struct sgrp *pwd)
{
        char **memb = NULL;
        int first = 1;

        printf("%s:%s:", pwd->sg_namp, pwd->sg_passwd);
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
#endif

static void print_passwd_info(struct passwd *pwd)
{
        printf("%s:%s:%u:%u:", pwd->pw_name, pwd->pw_passwd, pwd->pw_uid, pwd->pw_gid);
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

static void print_group_info(struct group *grp)
{
        char **memb = NULL;
        int first = 1;

        printf("%s:%s:%u:", grp->gr_name, grp->gr_passwd, grp->gr_gid);
        for (memb = grp->gr_mem; *memb != NULL; memb++) {
                printf("%s%s", first == 0 ? "," : "", *memb);
                first = 0;
        }
        printf("\n");
}

static int is_numeric(const char *v)
{
        if (v == NULL)
                return 1;

        for (; v != NULL && *v != (char)0; v++)
                if (*v < '0' || *v > '9')
                        return 0;

        return 1;
}

static void print_netent_info(struct netent *net)
{
        unsigned char *tmp = (unsigned char *)&net->n_net;
        int cnt = 0;

        cnt += printf("%s ", net->n_name);
        print_align_to(cnt, network_align_to);
        printf("%hhu.%hhu.%hhu.%hhu\n", tmp[3], tmp[2], tmp[1], tmp[0]);
}

static int get_networks(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++) {
                struct netent *net = NULL;

                net = getnetbyname(*keys);
                if (net == NULL) {
                        char dst[sizeof(uint32_t)];
                        uint32_t addr = 0;

                        /* TODO Find better way to get uint32_t network address */
                        if (inet_pton(AF_INET, *keys, dst) != 1)
                                return RES_KEY_NOT_FOUND;
                        addr = htonl(*(uint32_t *)dst);
                        net = getnetbyaddr(addr, AF_INET);
                }
                if (net != NULL)
                        print_netent_info(net);
        }

        return RES_OK;
}

#if HAVE_RPC
static void print_rpcent_info(struct rpcent *rpc)
{
        char **alias = NULL;
        int cnt = 0;
        int first = 1;

        cnt += printf("%s ", rpc->r_name);
        print_align_to(cnt, rpc_align_to);
        printf("%d", rpc->r_number);

        for (alias = rpc->r_aliases; alias != NULL && *alias != NULL; alias++) {
                printf("%s%s", first == 1 ? "  " : " ", *alias);
                first = 0;
        }
        printf("\n");
}
#endif

static void print_protoent_info(struct protoent *ent)
{
        char **alias = NULL;
        int cnt = 0;

        cnt += printf("%s ", ent->p_name);
        print_align_to(cnt, proto_align_to);
        printf("%d", ent->p_proto);
        for (alias = ent->p_aliases; alias != NULL && *alias != NULL; alias++) {
                printf(" %s", *alias);
        }
        printf("\n");
}

static void print_servent_info(struct servent *ent)
{
        char **alias = NULL;
        int cnt = 0;

        cnt = printf("%s ", ent->s_name);
        print_align_to(cnt, proto_align_to);
        printf("%hu/%s", ntohs((uint16_t)ent->s_port), ent->s_proto);
        for (alias = ent->s_aliases; alias != NULL && *alias != NULL; alias++) {
                printf(" %s", *alias);
        }
        printf("\n");
}

static int get_services(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++) {
                struct servent *ent = NULL;

                if (*keys == NULL)
                        continue;
                if (is_numeric(*keys) == 1)
                        ent = getservbyport(htons((uint16_t)atoi(*keys)), NULL);
                else
                        ent = getservbyname(*keys, NULL);
                if (ent != NULL)
                        print_servent_info(ent);
        }

        return RES_OK;
}

static int get_initgroups(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++) {
                gid_t *groups = calloc(MAX_GROUP_CNT, sizeof(gid_t));
                int group_cnt = (int)MAX_GROUP_CNT;
                int i = 0;
                int cnt = 0;

                if (groups == NULL)
                        err("Out of memory");

                if (getgrouplist(*keys, 0, groups, &group_cnt) == -1) {
                        free(groups);
                        return RES_KEY_NOT_FOUND;
                }
                cnt += printf("%s ", *keys);
                print_align_to(cnt, initgroup_align_to);
                for (i = 0; i < group_cnt; i++) {
                        if (groups != NULL && groups[i] > 0)
                                printf("%u ", groups[i]);
                }
                printf("\n");
                free(groups);
        }

        return RES_OK;
}

#if HAVE_NETGROUP
static void print_getent(const char *host, const char *user, const char *domain)
{
        if (host == NULL)
                return;
        printf("(%s,%s,%s)", host, user != NULL ? user : "", domain != NULL ? domain : "");
}

static int get_netgroup(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        if (key_cnt == 1) {
                char *host = NULL;
                char *user = NULL;
                char *domain = NULL;
                int first = 1;

                setnetgrent(*keys);
                do {
                        if (getnetgrent(&host, &user, &domain) == 0)
                                break;
                        if (first == 1) {
                                int cnt;

                                cnt = printf("%s ", *keys);
                                print_align_to(cnt, netgroup_align_to);
                                first = 0;
                        } else
                                printf(" ");
                        print_getent(host, user, domain);
                } while (host != NULL);
                if (first != 1)
                        printf("\n");
        } else if (key_cnt >= 4) {
                int res = innetgr(keys[0], keys[1], keys[2], keys[3]);
                int cnt = printf("%s ", *keys);

                print_align_to(cnt, netgroup_align_to);
                print_getent(keys[1], keys[2], keys[3]);
                printf(" = %d\n", res);
        } else
                return RES_KEY_NOT_FOUND;

        return RES_OK;
}
#endif

static int get_hosts(const char **keys, int key_cnt)
{
        return _get_hosts(keys, key_cnt, HOSTS_HOST);
}

static int get_ahosts(const char **keys, int key_cnt)
{
        return _get_hosts(keys, key_cnt, HOSTS_AHOST);
}

static int get_ahostsv4(const char **keys, int key_cnt)
{
        return _get_hosts(keys, key_cnt, HOSTS_AHOST_V4);
}

static int get_ahostsv6(const char **keys, int key_cnt)
{
        return _get_hosts(keys, key_cnt, HOSTS_AHOST_V6);
}

static int no_enum(const char *db)
{
        printf("Enumeration not supported on %s\n", db);
        return RES_ENUMERATION_NOT_SUPPORTED;
}

#if HAVE_ALIASES
ENUM_ALL(aliases, aliasent, , aliasent)
#endif
ENUM_ALL(services, servent, 1, servent)
ENUM_ALL(group, grent, , group)
#if HAVE_GSHADOW
ENUM_ALL(gshadow, sgent, , sgrp)
#endif
ENUM_ALL(hosts, hostent, 1, hostent)
ENUM_ALL(networks, netent, 1, netent)
ENUM_ALL(password, pwent, , passwd)
ENUM_ALL(protocols, protoent, 1, protoent)
#if HAVE_RPC
ENUM_ALL(rpc, rpcent, 1, rpcent)
#endif
ENUM_ALL(shadow, spent, , spwd)

#if HAVE_ALIASES
GET_SIMPLE(aliases, getaliasbyname, aliasent)
#endif
GET_NUMERIC_CAST(group, getgrnam, getgrgid, group, gid_t)
#if HAVE_GSHADOW
GET_SIMPLE(gshadow, getsgnam, sgrp)
#endif
GET_SIMPLE(password, getpwnam, passwd)
GET_NUMERIC(protocols, getprotobyname, getprotobynumber, protoent)
#if HAVE_RPC
GET_NUMERIC(rpc, getrpcbyname, getrpcbynumber, rpcent)
#endif
GET_SIMPLE(shadow, getspnam, spwd)

NO_ENUM_ALL_FOR(ethers)
NO_ENUM_ALL_FOR(initgroups)
#if HAVE_NETGROUP
NO_ENUM_ALL_FOR(netgroup)
#endif

static const getconf_database_config_t databases[] = {
        DATABASE_CONF_HOSTS(ahosts), DATABASE_CONF_HOSTS(ahostsv4), DATABASE_CONF_HOSTS(ahostsv6),
        DATABASE_CONF_HOSTS(hosts),
#if HAVE_ALIASES
        DATABASE_CONF(aliases),
#endif
        DATABASE_CONF(ethers),       DATABASE_CONF(group),
#if HAVE_GSHADOW
        DATABASE_CONF(gshadow),
#endif
        DATABASE_CONF(initgroups),
#if HAVE_NETGROUP
        DATABASE_CONF(netgroup),
#endif
        DATABASE_CONF(networks),     DATABASE_CONF(password),       DATABASE_CONF(protocols),
#if HAVE_RPC
        DATABASE_CONF(rpc),
#endif
        DATABASE_CONF(services),     DATABASE_CONF(shadow),
};
static const size_t databases_size = sizeof(databases) / sizeof(getconf_database_config_t);

static int read_database(const char *dbase, const char **keys, int key_cnt)
{
        size_t i = 0;

        for (i = 0; i < databases_size; i++) {
                if (strcmp(databases[i].name, dbase) != 0)
                        continue;
                if (keys != NULL)
                        return databases[i].get(keys, key_cnt);
                return databases[i].enum_all();
        }

        err("Unknown database: %s\n", dbase);
        return RES_MISSING_ARG_OR_INVALID_DATABASE;
}

/**
 * Program arguments.
 */
static struct option prog_opts[] = {
        { "service", optional_argument, 0, 's' },
        {
            "version",
            no_argument,
            NULL,
            'V',
        },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 },
};

/**
 * Print correct CLI usage of the tool
 */
static void printUsage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-i] [-s config] database [key ...]\n", progname);
}

/**
 * Pretty-print a help message
 */
static void printHelp(const char *progname)
{
        printUsage(progname);

        fputs("    -i, --no-idn                         Disable IDN encoding for lookups\n",
              stderr);
        fputs("    -s, --service=CONFIG                 Service configuration to be used\n",
              stderr);
        fputs("    -V, --version                        Display program version and quit\n",
              stdout);
}

/**
 * Print our version information
 */
static void printVersion(void)
{
        fputs("getent version " PACKAGE_VERSION " \n\n", stdout);
        fputs("Copyright © 2020 Serpent OS Developers\n", stdout);
        fputs("Part of the libc-support project\n", stdout);
        fputs("Available under the terms of the MIT license\n", stdout);
}

int main(int argc, char **argv)
{
        const char *dbase = NULL;
        const char **keys = NULL;
        int opt = 0;
        bool process_loop = true;
        __attribute__((unused)) bool idn = true;
        __attribute__((unused)) const char *service = NULL;
        const char *progname = argv[0];

        setlocale(LC_ALL, "");

        while (process_loop) {
                int option_index = 0;
                opt = getopt_long(argc, argv, "ahVs:i", prog_opts, &option_index);

                switch (opt) {
                case 'h':
                        printHelp(progname);
                        return EXIT_SUCCESS;
                        break;
                case 'V':
                        printVersion();
                        return EXIT_SUCCESS;
                case -1:
                        process_loop = false;
                        break;
                case '?':
                        printUsage(progname);
                        return EXIT_FAILURE;
                case 'i':
                        idn = false;
                        break;
                case 's':
                        service = optarg;
                        break;
                default:
                        break;
                }

                if (!process_loop) {
                        break;
                }
        }

        argc -= optind;
        argv += optind;

        dbase = *argv;
        --argc;
        if (argc > 0)
                keys = (const char **)++argv;

        if (dbase == NULL) {
                printUsage(progname);
                return RES_MISSING_ARG_OR_INVALID_DATABASE;
        }
        return read_database(dbase, keys, argc);
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 softtabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */
