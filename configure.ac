AC_INIT([libc-support], 1.2.0, [ikey@serpentos.com], [libc-support], [ikey@serpentos.com])
AM_INIT_AUTOMAKE([-Wno-portability no-dist-gzip dist-xz foreign subdir-objects])
AC_PROG_CC
AC_PROG_CC_STDC
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT(/usr/local)
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

AC_MSG_RESULT([
        libc-support $VERSION
        ========

        prefix:                 ${prefix}
        libdir:                 ${libdir}
        sysconfdir:             ${sysconfdir}
        exec_prefix:            ${exec_prefix}
        bindir:                 ${bindir}
        datarootdir:            ${datarootdir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}
])
