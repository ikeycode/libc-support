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
static const int network_align_to = 23;
static const int rpc_align_to = 17;
static const int proto_align_to = 23;
static const int initgroup_align_to = 23;
static const int netgroup_align_to = 23;

#define DST_LEN 256
static const size_t MAX_GROUP_CNT = 256;

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

enum {
        RES_OK = 0,
        RES_MISSING_ARG_OR_INVALID_DATABASE = 1,
        RES_KEY_NOT_FOUND = 2,
        RES_ENUMERATION_NOT_SUPPORTED = 3,
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

typedef int (*get_func_t)(const char **keys, int key_cnt);
typedef int (*get_all_func_t)(void);

typedef struct getconf_database_config {
        const char *name;
        get_func_t get;
        get_all_func_t get_all;
} getconf_database_config_t;

#define DATABASE_CONF(X) { #X, get_ ## X, get_ ## X ## _all }
#define DATABASE_CONF_HOSTS(X) { #X, get_ ## X, get_hosts_all }

#define GET_ALL(X, single, base, initparm, type) \
static int get_ ## X ## _all(void)\
{\
        struct type *ent = NULL;\
        set ## base(initparm);\
        while ((ent = get ## base()) != NULL)\
                print_ ## single ## _info(ent);\
        end ## base();\
        return RES_OK;\
}

static void err(const char *msg, ...)
{
        va_list args;

        va_start(args, msg);
        (void)vfprintf(stderr, msg, args);
        va_end(args);

        exit(EXIT_FAILURE);
}

static void usage(const char *name, int help)
{
        fprintf(stderr, "Usage: %s database [key ...]\n", name);

        if (help == HELP_FULL) {
                fprintf(stderr, "\nArguments:\n");
                fprintf(stderr, "  -i, --no-idn           Disable IDN (not implemented)\n");
                fprintf(stderr, "  -s, --service=CONFIG   Service configuration to be used (not implemented)\n");
                fprintf(stderr, "  database               Database to query from\n");
                fprintf(stderr, "  key                    Key to query\n");
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
                cnt += printf("%s%hhu", first == 0 ? "." : "", (unsigned char)*addr);
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
                cnt += printf("%s ", inet_ntop(AF_INET, (const void *)&sin->sin_addr, (char *)dst, DST_LEN));
        } else {
                struct sockaddr_in6 *sin = (struct sockaddr_in6 *)addr;
                cnt += printf("%s ", inet_ntop(AF_INET6, (const void *)&sin->sin6_addr, (char *)dst, DST_LEN));
        }
        print_align_to(cnt, align_to);
        if (sock_type > 0 && (size_t)sock_type < socktype_size)
                printf("%s", socktypes[sock_type]);
        if (print_host == 0) {
                printf("\n");
                return;
        }
        (void)getnameinfo(addr, family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, 0);
        host[NI_MAXHOST- 1] = 0;
        printf("%s\n", host);
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
}

static int _get_hosts(const char **keys, int key_cnt, int host_type)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++)
                print_single_host_info(*keys, host_type);

        return RES_OK;
}

static void print_host_info(struct hostent *ent)
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

static int get_aliases(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++) {
                struct aliasent *ent = NULL;

                ent = getaliasbyname(*keys);
                if (ent != NULL)
                        print_alias_info(ent);
        }

        return RES_OK;
}

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

static int get_shadow(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++) {
                struct spwd *pwd = NULL;

                pwd = getspnam(*keys);
                if (pwd != NULL)
                        print_shadow_info(pwd);
        }

        return RES_OK;
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

static int get_gshadow(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++) {
                struct sgrp *pwd = NULL;

                pwd = getsgnam(*keys);
                if (pwd != NULL)
                        print_gshadow_info(pwd);
        }

        return RES_OK;
}

static void print_password_info(struct passwd *pwd)
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

static int get_password(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++) {
                struct passwd *pwd = NULL;

                pwd = getpwnam(*keys);
                if (pwd != NULL)
                        print_password_info(pwd);
        }

        return RES_OK;
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
        if (v == NULL)
                return 1;
        for (; v != NULL && *v != (char)0; v++)
                if (*v < '0' || *v > '9')
                        return 0;

        return 1;
}

static int get_group(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return 1;

        for (; key_cnt-- > 0; keys++) {
                struct group *grp = NULL;

                if (*keys == NULL)
                        continue;
                if (is_numeric(*keys) == 1)
                        grp = getgrgid(atoi(*keys));
                else
                        grp = getgrnam(*keys);
                if (grp != NULL)
                        print_group_info(grp);
        }

        return RES_OK;
}

static void print_network_info(struct netent *net)
{
        unsigned char *tmp = (unsigned char*)&net->n_net;
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
                        addr = htonl(*(uint32_t*)dst);
                        net = getnetbyaddr(addr, AF_INET);
                }
                if (net != NULL)
                        print_network_info(net);
        }

        return RES_OK;
}

static void print_rpc_info(struct rpcent *rpc)
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

static int get_rpc(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++) {
                struct rpcent *rpc = NULL;

                if (*keys == NULL)
                        continue;
                if (is_numeric(*keys) == 1)
                        rpc = getrpcbynumber(atoi(*keys));
                else
                        rpc = getrpcbyname(*keys);
                if (rpc != NULL)
                        print_rpc_info(rpc);
        }

        return RES_OK;
}

static void print_protocol_info(struct protoent *ent)
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

static int get_protocols(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++) {
                struct protoent *ent = NULL;

                if (*keys == NULL)
                        continue;
                if (is_numeric(*keys) == 1)
                        ent = getprotobynumber(atoi(*keys));
                else
                        ent = getprotobyname(*keys);
                if (ent != NULL)
                        print_protocol_info(ent);
        }

        return RES_OK;
}

static void print_service_info(struct servent *ent)
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
                        print_service_info(ent);
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

static void print_getent(const char *host, const char *user, const char *domain)
{
        if (host == NULL)
                return;
        printf("(%s,%s,%s)", host,
                user != NULL ? user : "",
                domain != NULL ? domain : "");
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
        return _get_hosts(keys, key_cnt, HOSTS_AHOST_V4);
}

static int no_enum(const char *db)
{
        printf("Enumeration not supported on %s\n", db);
        return RES_ENUMERATION_NOT_SUPPORTED;
}

GET_ALL(aliases, alias, aliasent, ,aliasent)
GET_ALL(services, service, servent, 1, servent)
GET_ALL(group, group, grent, , group)
GET_ALL(gshadow, gshadow, sgent, , sgrp)
GET_ALL(hosts, host, hostent, 1, hostent)
GET_ALL(networks, network, netent, 1, netent)
GET_ALL(password, password, pwent, , passwd)
GET_ALL(protocols, protocol, protoent, 1, protoent)
GET_ALL(rpc, rpc, rpcent, 1, rpcent)
GET_ALL(shadow, shadow, spent, , spwd)

#define NO_ENUM_ALL_FOR(X) \
static int get_ ## X ## _all(void)\
{\
        return no_enum(#X);\
}

NO_ENUM_ALL_FOR(ethers)
NO_ENUM_ALL_FOR(initgroups)
NO_ENUM_ALL_FOR(netgroup)

static const getconf_database_config_t databases[] = {
        DATABASE_CONF_HOSTS(ahosts),
        DATABASE_CONF_HOSTS(ahostsv4),
        DATABASE_CONF_HOSTS(ahostsv6),
        DATABASE_CONF_HOSTS(hosts),
        DATABASE_CONF(aliases),
        DATABASE_CONF(ethers),
        DATABASE_CONF(group),
        DATABASE_CONF(gshadow),
        DATABASE_CONF(initgroups),
        DATABASE_CONF(netgroup),
        DATABASE_CONF(networks),
        DATABASE_CONF(password),
        DATABASE_CONF(protocols),
        DATABASE_CONF(rpc),
        DATABASE_CONF(services),
        DATABASE_CONF(shadow),
};
static const size_t databases_size = sizeof(databases) / sizeof (getconf_database_config_t);

static int read_database(const char *dbase, const char **keys, int key_cnt)
{
        size_t i = 0;

        for (i = 0; i < databases_size; i++) {
                if (strcmp(databases[i].name, dbase) != 0)
                        continue;
                if (keys != NULL)
                        return databases[i].get(keys, key_cnt);
                return databases[i].get_all();
        }

        err("Unknown database: %s\n", dbase);
        return RES_MISSING_ARG_OR_INVALID_DATABASE;
}

int main(int argc, char **argv)
{
        const char *dbase = NULL;
        const char **keys = NULL;
        int index = 1;

        for (index = 1; index < argc; index++) {
                if (strcmp("-h", argv[index]) == 0)
                        usage(argv[0], HELP_FULL);
                else if (strcmp("-s", argv[index]) == 0)
                        index++;
                /* Ignored for now */
                else if(strncmp("--service", argv[index], 9) == 0) {
                        /* Ignored for now */
                } else if (strcmp("-i", argv[index]) == 0 || strcmp("--no-idn", argv[index]) == 0) {
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

        return read_database(dbase, keys, argc - index);
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
