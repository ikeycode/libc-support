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

#include <pwd.h>
#include <stdio.h>

#include "getent.h"

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

GET_SIMPLE(password, getpwnam, passwd)
ENUM_ALL(password, pwent, , passwd)

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
