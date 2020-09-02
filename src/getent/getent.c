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

#include <getopt.h>
#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "databases.h"
#include "getent.h"

enum { HELP_SHORT, HELP_FULL };

void err(const char *msg, ...)
{
        va_list args;

        va_start(args, msg);
        (void)vfprintf(stderr, msg, args);
        va_end(args);

        exit(EXIT_FAILURE);
}

int is_numeric(const char *v)
{
        if (v == NULL)
                return 1;

        for (; v != NULL && *v != (char)0; v++)
                if (*v < '0' || *v > '9')
                        return 0;

        return 1;
}

static int read_database(const char *dbase, const char **keys, int key_cnt)
{
        size_t i = 0;

        for (i = 0; i < databases_size; i++) {
                if (databases[i].name == NULL)
                        continue;
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
        fprintf(stdout, "Usage: %s [-i] [-s config] database [key ...]\n", progname);
}

/**
 * Pretty-print a help message
 */
static void printHelp(const char *progname)
{
        printUsage(progname);

        fputs("    -i, --no-idn                         Disable IDN encoding for lookups\n",
              stdout);
        fputs("    -s, --service=CONFIG                 Service configuration to be used\n",
              stdout);
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
