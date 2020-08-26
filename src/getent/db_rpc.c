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

#if HAVE_RPC

static const int rpc_align_to = 17;

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

GET_NUMERIC(rpc, getrpcbyname, getrpcbynumber, rpcent)
ENUM_ALL(rpc, rpcent, 1, rpcent)

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
