#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
        const char *name;
        int key;
} conf_var_t;

static const conf_var_t sysconf_vars[] = {
        /* POSIX.1 variables from man sysconf */
        { "ARG_MAX", _SC_ARG_MAX },
        { "CHILD_MAX", _SC_CHILD_MAX },
        { "HOST_NAME_MAX", _SC_HOST_NAME_MAX },
        { "LOGIN_NAME_MAX", _SC_LOGIN_NAME_MAX },
        { "NGROUPS_MAX", _SC_NGROUPS_MAX },
        { "OPEN_MAX", _SC_OPEN_MAX },
        { "PAGESIZE", _SC_PAGESIZE },
        { "RE_DUP_MAX", _SC_RE_DUP_MAX },
        { "STREAM_MAX", _SC_STREAM_MAX },
        { "SYMLOOP_MAX", _SC_SYMLOOP_MAX },
        { "TTY_NAME_MAX", _SC_TTY_NAME_MAX },
        { "TZNAME_MAX", _SC_TZNAME_MAX },
        { "_POSIX_VERSION", _SC_VERSION },

        /* POSIX.2 variables from man sysconf */
        { "BC_BASE_MAX", _SC_BC_BASE_MAX },
        { "BC_DIM_MAX", _SC_BC_DIM_MAX },
        { "BC_SCALE_MAX", _SC_BC_SCALE_MAX },
        { "COLL_WEIGHTS_MAX", _SC_COLL_WEIGHTS_MAX },
        { "EXPR_NEST_MAX", _SC_EXPR_NEST_MAX },
        { "LINE_MAX", _SC_LINE_MAX },
        { "RE_DUP_MAX", _SC_RE_DUP_MAX },
        { "POSIX2_VERSION", _SC_2_VERSION },
        { "POSIX2_C_DEV", _SC_2_C_DEV },
        { "POSIX2_FORT_DEV", _SC_2_FORT_DEV },
        { "POSIX2_FORT_RUN", _SC_2_FORT_RUN },
        { "_POSIX2_LOCALEDEF", _SC_2_LOCALEDEF },
        { "POSIX2_SW_DEV", _SC_2_SW_DEV },

        /* Others spotted from "getconf -a" list */
        { "ATEXIT_MAX", _SC_ATEXIT_MAX },
        { "CHAR_BIT", _SC_CHAR_BIT },
        { "CHAR_MIN", _SC_CHAR_MIN },
        { "CHAR_MAX", _SC_CHAR_MAX },
        { "SHRT_MIN", _SC_SHRT_MIN },
        { "SHRT_MAX", _SC_SHRT_MAX },
        { "INT_MIN", _SC_INT_MIN },
        { "INT_MAX", _SC_INT_MAX },
        { "UCHAR_MAX", _SC_UCHAR_MAX },
        { "USHRT_MAX", _SC_USHRT_MAX },
        { "UINT_MAX", _SC_UINT_MAX },
        { "ULONG_MAX", _SC_ULONG_MAX },
        { "WORD_BIT", _SC_WORD_BIT },
        { "IOV_MAX", _SC_IOV_MAX },
};

static const conf_var_t confstr_vars[] = {
        { "PATH", _CS_PATH },
};

/* Variables from man pathconf */
static const conf_var_t pathconf_vars[] = {
        { "LINK_MAX", _PC_LINK_MAX },
        { "MAX_CANON", _PC_MAX_CANON },
        { "MAX_INPUT", _PC_MAX_INPUT },
        { "NAME_MAX", _PC_NAME_MAX },
        { "PATH_MAX", _PC_PATH_MAX },
        { "PIPE_BUF", _PC_PIPE_BUF },
        { "CHOWN_RESTRICTED", _PC_CHOWN_RESTRICTED },
        { "NO_TRUNC", _PC_NO_TRUNC },
        { "VDISABLE", _PC_VDISABLE },
};

static const size_t sysconf_var_cnt = sizeof(sysconf_vars) / sizeof(conf_var_t);
static const size_t confstr_var_cnt = sizeof(confstr_vars) / sizeof(conf_var_t);
static const size_t pathconf_var_cnt = sizeof(pathconf_vars) / sizeof(conf_var_t);

static const long text_align_to_chars = 25;
static const int NOT_IN_LIST = -1;

enum { HELP_SHORT, HELP_FULL };

static void err(const char *msg, ...)
{
        va_list args;

        /*@-nullpass@*/
        va_start(args, msg);
        (void)vfprintf(stderr, msg, args);
        va_end(args);
        /*@=nullpass@*/

        exit(EXIT_FAILURE);
}

static int print_sysconf(int val)
{
        long res = sysconf(val);

        if (val == _SC_ULONG_MAX)
                printf("%lu\n", (unsigned long)res);
        else if (res == -1)
                printf("undefined\n");
        else
                printf("%ld\n", res);

        return 0;
}

static int print_confstr(int val)
{
        /*@-nullpass@*/
        size_t len = confstr(val, NULL, 0);
        /*@=nullpass@*/
        char *res = NULL;

        if (len == 0) {
                printf("undefined\n");
                return 0;
        }

        res = calloc(1, len + 1);
        if (res == NULL)
                err("Out of memory");
        else if (confstr(val, res, len) == len)
                printf("%s\n", res);
        else
                printf("undefined\n");
        free(res);

        return 0;
}

static int print_pathconf(int val, /*@null@*/ const char *path)
{
        long res = 0;

        if (path == NULL)
                return 0;

        res = pathconf(path, val);
        if (res == -1)
                printf("undefined\n");
        else
                printf("%ld\n", res);

        return 0;
}

static int in_list(const conf_var_t *vars, size_t cnt, /*@null@*/ const char *var)
{
        const conf_var_t *max_item = vars + cnt;
        const conf_var_t *item = NULL;

        if (var == NULL || vars == NULL)
                return NOT_IN_LIST;

        for (item = vars; item < max_item; item++) {
                if (item->name != NULL && strcmp(item->name, var) == 0)
                        return item->key;
        }

        return NOT_IN_LIST;
}

static int print_variable(/*@null@*/ const char *var, /*@null@*/ const char *path)
{
        int res = 0;

        if (var == NULL)
                err("No variable\n");

        res = in_list(sysconf_vars, sysconf_var_cnt, var);
        if (res != NOT_IN_LIST)
                return print_sysconf(res);

        res = in_list(confstr_vars, confstr_var_cnt, var);
        if (res != NOT_IN_LIST)
                return print_confstr(res);

        if (path != NULL) {
                res = in_list(pathconf_vars, pathconf_var_cnt, var);
                if (res != NOT_IN_LIST)
                        return print_pathconf(res, path);
        }

        /*@-nullpass@*/
        err("Unrecognized variable: '%s'\n", var);
        /*@=nullpass@*/

        return 1;
}

static void print_aligned_to(const char *msg, long align_to)
{
        long cnt = 0;

        if (msg == NULL)
                return;

        printf("%s ", msg);
        cnt = align_to - (long)strlen(msg);
        if (cnt <= 0)
                return;

        while (cnt-- > 0) {
                if (fputc(' ', stdout) == EOF)
                        break;
        }
}

static int print_all(/*@null@*/ const char *path)
{
        const conf_var_t *item = NULL;

        for (item = sysconf_vars; item < sysconf_vars + sysconf_var_cnt; item++) {
                print_aligned_to(item->name, text_align_to_chars);
                (void)print_sysconf(item->key);
        }
        for (item = confstr_vars; item < confstr_vars + confstr_var_cnt; item++) {
                print_aligned_to(item->name, text_align_to_chars);
                (void)print_confstr(item->key);
        }
        for (item = pathconf_vars; item < pathconf_vars + pathconf_var_cnt; item++) {
                print_aligned_to(item->name, text_align_to_chars);
                (void)print_pathconf(item->key, path != NULL ? path : ".");
        }

        return 0;
}

static void usage(const char *name, int help)
{
        fprintf(stderr, "Usage: %s [-v specification] variable [path]\n", name);
        fprintf(stderr, "       %s -a [path]\n", name);

        if (help == HELP_FULL) {
                fprintf(stderr, "\nArguments:\n");
                fprintf(stderr,
                        "  -a           Display all configuration variables and their values\n");
                fprintf(stderr,
                        "  -v spec      Indicate the specification and version to obtain "
                        "configuration\n");
                fprintf(stderr, "               variable for\n");
                fprintf(stderr, "  variable     The variable name to get value for\n");
                fprintf(stderr,
                        "  path         File system path to get pathconf(3) variable from\n");
        }

        exit(EXIT_FAILURE);
}

int main(int argc, const char **argv)
{
        const char *varname = NULL;
        const char *pathname = NULL;
        int all = 0;
        int index = 1;

        if (argc <= 1)
                usage(argv[0], HELP_SHORT);

        for (index = 1; index < argc; index++) {
                if (strcmp("-a", argv[index]) == 0)
                        all = 1;
                else if (strcmp("-h", argv[index]) == 0 || strcmp("--help", argv[index]) == 0)
                        usage(argv[0], HELP_FULL);
                else if (strcmp("-v", argv[index]) == 0) {
                        index++;
                        if (index >= argc)
                                usage(argv[0], HELP_SHORT);
                        /* Value ignored for now */
                } else if (all == 0 && varname == NULL)
                        varname = argv[index];
                else if (pathname == NULL)
                        pathname = argv[index];
                else
                        err("Invalid argument: %s\n", argv[index]);
        }

        if (all == 1)
                return print_all(pathname);

        return print_variable(varname, pathname);
}
