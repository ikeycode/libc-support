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
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "getent.h"

enum { HOSTS_HOST,
       HOSTS_AHOST,
       HOSTS_AHOST_V4,
       HOSTS_AHOST_V6,
};

/* From bits/socket_type.h */
static const char *socktypes[] = {
        [SOCK_STREAM] = "STREAM", [SOCK_DGRAM] = "DGRAM",         [SOCK_RAW] = "RAW",
        [SOCK_RDM] = "RDM",       [SOCK_SEQPACKET] = "SEQPACKET", [SOCK_DCCP] = "DCCP",
        [SOCK_PACKET] = "PACKET"
};

static const size_t socktype_size = sizeof(socktypes) / sizeof(const char *);
static const int addr_align_to = 16;

void print_hostent_info(struct hostent *ent)
{
        char **aliases = ent->h_aliases;
        char dst[DST_LEN];
        int cnt = 0;

        inet_ntop(ent->h_addrtype, ent->h_addr_list[0], (char *)dst, DST_LEN);
        cnt = printf("%s", dst);
        print_align_to(cnt, addr_align_to);
        printf(" %s", ent->h_name);
        while (aliases != NULL && *aliases != NULL) {
                printf(" %s", *aliases);
                aliases++;
        }
        printf("\n");
}

static void print_sockaddr(struct sockaddr *addr, int family, int sock_type, int print_host)
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
        print_align_to(cnt, addr_align_to);
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
                        print_sockaddr(tmp->ai_addr, tmp->ai_family, tmp->ai_socktype, print_host);
                        print_host = 0;
                }
        } else
                print_sockaddr(info->ai_addr, info->ai_family, 0, 1);

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

int get_hosts(const char **keys, int key_cnt)
{
        return _get_hosts(keys, key_cnt, HOSTS_HOST);
}

int get_ahosts(const char **keys, int key_cnt)
{
        return _get_hosts(keys, key_cnt, HOSTS_AHOST);
}

int get_ahostsv4(const char **keys, int key_cnt)
{
        return _get_hosts(keys, key_cnt, HOSTS_AHOST_V4);
}

int get_ahostsv6(const char **keys, int key_cnt)
{
        return _get_hosts(keys, key_cnt, HOSTS_AHOST_V6);
}

ENUM_ALL(ahostsv4, hostent, 1, hostent)
ENUM_ALL(ahostsv6, hostent, 1, hostent)
ENUM_ALL(ahosts, hostent, 1, hostent)
ENUM_ALL(hosts, hostent, 1, hostent)

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
