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
#include <netinet/ether.h>
#include <string.h>

int get_ethers(const char **keys, int key_cnt)
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

NO_ENUM_ALL_FOR(ethers)

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
