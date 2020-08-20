#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    const char *name;
    int key;
} conf_var_t;

/* Variables from man sysconf */
const conf_var_t sysconf_vars[] = {
    /* POSIX.1 */
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

    /* POSIX.2 */
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
};

const conf_var_t confstr_vars[] = {
    { "PATH", _CS_PATH },
};

/* Variables from man pathconf */
const conf_var_t pathconf_vars[] = {
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

const size_t sysconf_var_cnt = sizeof(sysconf_vars) / sizeof(conf_var_t);
const size_t confstr_var_cnt = sizeof(confstr_vars) / sizeof(conf_var_t);
const size_t pathconf_var_cnt = sizeof(pathconf_vars) / sizeof(conf_var_t);

void err(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    exit(1);
}

int print_sysconf(int val)
{
    long int res = sysconf(val);

    if (res == -1) {
        printf("undefined\n");
    } else {
        printf("%ld\n", res);
    }
    return 0;
}

int print_confstr(int val)
{
    size_t len = confstr(val, NULL, 0);
    char *res;

    if (len == 0) {
        printf("undefined\n");
        return 0;
    }

    res = calloc(1, len + 1);
    if (!res)
        err("Out of memory");
    confstr(val, res, len);
    printf("%s\n", res);
    free(res);
    return 0;
}

int print_pathconf(int val, const char *path)
{
    long int res = pathconf(path, val);

    if (res == -1) {
        printf("undefined\n");
    } else {
        printf("%ld\n", res);
    }
    return 0;
}


int in_list(const conf_var_t *vars, size_t cnt, const char *var)
{
    const conf_var_t *item = vars;
    const conf_var_t *max_item = vars + cnt;

    if (!var)
        return -1;

    for (; item < max_item; item++) {
        if (item->name && strcmp(item->name, var) == 0) {
            return item->key;
        }
    }

    return -1;
}

int print_variable(const char *var, const char *path)
{
    if (!var)
        err("No variable\n");

    int res = in_list(sysconf_vars, sysconf_var_cnt, var);
    if (res >= 0)
        return print_sysconf(res);

    res = in_list(confstr_vars, confstr_var_cnt, var);
    if (res >= 0)
        return print_confstr(res);

    if (path) {
        res = in_list(pathconf_vars, pathconf_var_cnt, var);
        if (res >= 0)
            return print_pathconf(res, path);
    }

    err("Unrecognized variable: '%s'\n", var);
    return 1;
}

void usage(const char *name)
{
    fprintf(stderr, "Usage: %s [-v specification] variable_name\n", name);
    exit(1);
}

int main(int argc, const char **argv)
{
    const char *varname = NULL;
    const char *pathname = NULL;
    const char *spec = NULL;
    int index;

    if (argc <= 1)
        usage(argv[0]);

    for (index = 1; index < argc; index++) {
        if (strcmp("-v", argv[index]) == 0) {
            index++;
            if (index >= argc)
                usage(argv[0]);
            else
                spec = argv[index];
        } else if (!varname) {
            varname = argv[index];
        } else if (!pathname) {
            pathname = argv[index];
        } else
            err("Invalid argument: %s\n", argv[index]);
    }

    /* For now just ignore */
    (void)spec;

    return print_variable(varname, pathname);
}