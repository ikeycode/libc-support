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

#include <grp.h>
#include <stdlib.h>

static const int initgroup_align_to = 23;
static const size_t MAX_GROUP_CNT = 256;

int get_initgroups(const char **keys, int key_cnt)
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

NO_ENUM_ALL_FOR(initgroups)

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
