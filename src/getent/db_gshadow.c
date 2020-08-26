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

#if HAVE_GSHADOW
#include <gshadow.h>

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

GET_SIMPLE(gshadow, getsgnam, sgrp)
ENUM_ALL(gshadow, sgent, , sgrp)

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
