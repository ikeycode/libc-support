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
#include <stddef.h>
#include <stdio.h>

#include "getent.h"

static const int network_align_to = 23;

static void print_netent_info(struct netent *net)
{
        struct in_addr addr;
        int cnt = 0;
        char **aliases = net->n_aliases;

        cnt += printf("%s ", net->n_name);
        print_align_to(cnt, network_align_to);
        addr.s_addr = htonl(net->n_net);
        printf("%s", inet_ntoa(addr));
        while (aliases != NULL && *aliases != NULL) {
                printf(" %s", *aliases);
                aliases++;
        }
        printf("\n");
}

int get_networks(const char **keys, int key_cnt)
{
        if (keys == NULL)
                return RES_KEY_NOT_FOUND;

        for (; key_cnt-- > 0; keys++) {
                struct netent *net = NULL;

                net = getnetbyname(*keys);
                if (net == NULL) {
                        struct addrinfo *info = NULL;
                        struct sockaddr_in *sin = NULL;
                        int res = 0;

                        res = getaddrinfo(*keys, NULL, NULL, &info);
                        if (res != 0 || info == NULL)
                                return RES_KEY_NOT_FOUND;
                        sin = (struct sockaddr_in *)info->ai_addr;
                        net = getnetbyaddr(ntohl(sin->sin_addr.s_addr), AF_INET);
                }
                if (net != NULL)
                        print_netent_info(net);
        }

        return RES_OK;
}

ENUM_ALL(networks, netent, 1, netent)

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
