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

#include "getent.h"

#include <netdb.h>
#include <stdlib.h>

#if HAVE_NETGROUP
static const int netgroup_align_to = 23;

static void print_getent(const char *host, const char *user, const char *domain)
{
        if (host == NULL)
                return;
        printf("(%s,%s,%s)", host, user != NULL ? user : "", domain != NULL ? domain : "");
}

int get_netgroup(const char **keys, int key_cnt)
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

NO_ENUM_ALL_FOR(netgroup)

#endif

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
