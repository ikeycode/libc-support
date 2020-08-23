#define _GNU_SOURCE

#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

/**
 * How we'll find a given variable by name.
 */
typedef enum {
        LOOKUP_SYSCONF = 0,  /**< Lookup in sysconf() */
        LOOKUP_PATHCONF = 1, /**< Lookup in pathconf() */
        LOOKUP_CONFSTR = 2,  /**< Lookup in confstr() */
        LOOKUP_DEFINE = 3,   /**< Already defined, print it. */
} LookupMethod;

/**
 * SystemConfigVariable encapsulates various getconf variables, notably:
 *
 * - sysconf
 * - confstr
 * - pathconf
 *
 * We used a singular merged list of all variables to make it far easier
 * to maintain this utility.
 */
typedef struct SystemConfigVariable {
        const char *name; /**<Name of the variale */
        union {
                long long lkey; /**< Long key */
                short skey;     /**< Short key */
        };
        bool unsign;         /**< Signed/unsigned */
        LookupMethod method; /**< Where to retrieve variable */
} SystemConfigVariable;

/**
 * The variable is callable via sysconf() interface
 */
#define GET_SYSCONF_VARIABLE(N, K)                                                                 \
        {                                                                                          \
                .name = N, .skey = K, .method = LOOKUP_SYSCONF, .unsign = false                    \
        }

/**
 * The variable is compiler-defined, and is a signed number
 */
#define GET_SIGNED_DEFINITION(N, K)                                                                \
        {                                                                                          \
                .name = N, .lkey = K, .method = LOOKUP_DEFINE, .unsign = false                     \
        }

/**
 * The variable is compiler-defined, and is an unsigned number
 */
#define GET_UNSIGNED_DEFINITION(N, K)                                                              \
        {                                                                                          \
                .name = N, .lkey = (long long)K, .method = LOOKUP_DEFINE, .unsign = true           \
        }

/**
 * The variable is callable via confstr() interface
 */
#define GET_CONFSTR_VARIABLE(N, K)                                                                 \
        {                                                                                          \
                .name = N, .lkey = K, .method = LOOKUP_CONFSTR, .unsign = false                    \
        }

/**
 * The variable is callable via pathconf() interface
 */
#define GET_PATHCONF_VARIABLE(N, K)                                                                \
        {                                                                                          \
                .name = N, .lkey = K, .method = LOOKUP_PATHCONF, .unsign = false                   \
        }

/**
 * All of our variables come from getconf.inc to make it easier to
 * maintain
 */
static SystemConfigVariable system_config_vars[] = {
#include "getconf.inc"
};

/**
 * Return the length of an array with known storage size at compile time
 */
#define ARRAY_SIZE(v) sizeof(v) / sizeof(v[0])

/**
 * Print a single variable and its corresponding value
 */
static inline void print_one(const SystemConfigVariable *v, const char *filename)
{
        switch (v->method) {
        case LOOKUP_SYSCONF:
                fprintf(stdout, "%ld\n", sysconf(v->skey));
                break;
        case LOOKUP_DEFINE:
                if (v->unsign) {
                        fprintf(stdout, "%llu\n", (unsigned long long)v->lkey);
                } else {
                        fprintf(stdout, "%lld\n", v->lkey);
                }
                break;
        case LOOKUP_CONFSTR: {
                char *buffer = NULL;
                size_t sz = 0;

                sz = confstr((int)v->lkey, buffer, 0);
                if (sz <= 0) {
                        return;
                }
                /* Ensure we can allocate */
                buffer = calloc(sz, sizeof(char));
                if (!buffer) {
                        abort();
                }
                /* Should be the same size this time! */
                if (confstr((int)v->lkey, buffer, sz) != sz) {
                        free(buffer);
                        abort();
                }
                fprintf(stdout, "%s\n", buffer);
                free(buffer);
                break;
        }
        case LOOKUP_PATHCONF:
                fprintf(stdout, "%ld\n", pathconf(filename == NULL ? "." : filename, v->skey));
                break;
        default:
                fprintf(stdout, "\n");
        }
}

/**
 * Print all variables and their corresponding values
 */
static void print_all(const char *filename)
{
        size_t largest = 0;
        /* Compute largest name. */
        for (uint16_t i = 0; i < ARRAY_SIZE(system_config_vars); i++) {
                size_t newsize = strlen(system_config_vars[i].name);
                if (newsize > largest) {
                        largest = newsize;
                }
        }

        for (uint16_t i = 0; i < ARRAY_SIZE(system_config_vars); i++) {
                SystemConfigVariable *v = &system_config_vars[i];
                fprintf(stdout, "%-*s\t", (int)largest, v->name);
                print_one(v, filename);
        }
}

/**
 * Return the SystemConfigVariable for the given variable name.
 * Returns NULL ("undefined") if it cannot be found
 */
static const SystemConfigVariable *find_variable(const char *name)
{
        for (uint16_t i = 0; i < ARRAY_SIZE(system_config_vars); i++) {
                const SystemConfigVariable *var = &system_config_vars[i];
                if (var && var->name && name && strcmp(name, var->name) == 0) {
                        return var;
                }
        }
        return NULL;
}

/**
 * Program arguments.
 */
static struct option prog_opts[] = {
        { "version-specification", no_argument, 0, 'v' },
        {
            "version",
            no_argument,
            0,
            'V',
        },
        { "help", no_argument, 0, 'h' },
        { "all", no_argument, 0, 'a' },
        { NULL, 0, 0, 0 },
};

/**
 * Print correct CLI usage of the tool
 */
static void printUsage(const char *progname)
{
        fprintf(stdout, "Usage: %s [-v specification] variable_name [pathname]\n", progname);
        fprintf(stdout, "       %s -a [pathname]\n", progname);
}

/**
 * Pretty-print a help message
 */
static void printHelp(const char *progname)
{
        printUsage(progname);

        fputs("    -v, --version-specification          Set the version specification\n", stdout);
        fputs("    -h, --help                           Display this help message\n", stdout);
        fputs("    -a, --all                            Display all applicable variables\n",
              stdout);
        fputs("    -V, --version                        Display program version and quit\n",
              stdout);
}

/**
 * Print our version information
 */
static void printVersion(void)
{
        fputs("getconf version " PACKAGE_VERSION " \n\n", stdout);
        fputs("Copyright © 2020 Serpent OS Developers\n", stdout);
        fputs("Part of the libc-support project\n", stdout);
        fputs("Available under the terms of the MIT license\n", stdout);
}

/**
 * Main entry point into the program
 */
int main(int argc, char **argv)
{
        int opt = 0;
        __attribute__((unused)) const char *specification = NULL;
        const SystemConfigVariable *variable = NULL;
        bool process_loop = true;
        /* Stash before winding */
        const char *progname = argv[0];
        bool listing = false;

        while (process_loop) {
                int option_index = 0;
                opt = getopt_long(argc, argv, "ahVv:", prog_opts, &option_index);

                switch (opt) {
                case 'h':
                        printHelp(progname);
                        return EXIT_SUCCESS;
                        break;
                case 'v':
                        specification = optarg;
                        break;
                case 'V':
                        printVersion();
                        return EXIT_SUCCESS;
                case 'a':
                        listing = true;
                        break;
                case -1:
                        process_loop = false;
                        break;
                case '?':
                        printUsage(progname);
                        return EXIT_FAILURE;
                default:
                        break;
                }

                if (!process_loop) {
                        break;
                }
        }

        argc -= optind;
        argv += optind;

        /**
         * When listing, we accept only 1 argument, for pathconf() usage
         */
        if (listing) {
                const char *pathscan = NULL;
                if (argc == 1) {
                        pathscan = argv[0];
                } else if (argc > 1) {
                        printUsage(progname);
                        return EXIT_FAILURE;
                }

                print_all(pathscan);
                return EXIT_SUCCESS;
        }

        /**
         * Process according to the number of arguments. A filepath means
         * we'll only deal with PATHCONF variables.
         */
        switch (argc) {
        /* sysconf/defines/confstr */
        case 1:
                variable = find_variable(argv[0]);
                if (!variable) {
                        fputs("undefined\n", stdout);
                        return EXIT_SUCCESS;
                }
                print_one(variable, NULL);
                break;
        /* pathconf only */
        case 2:
                variable = find_variable(argv[0]);
                if (!variable) {
                        return EXIT_SUCCESS;
                }

                if (variable->method != LOOKUP_PATHCONF) {
                        fputs("undefined\n", stdout);
                        return EXIT_SUCCESS;
                }
                print_one(variable, argv[1]);
                break;
        default:
                printUsage(progname);
                break;
        }

        return 0;
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
 * vi: set shiftwidth=8 tabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */
