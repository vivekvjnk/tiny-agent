AC_PREREQ([2.69])
AC_INIT([tiny-agent], [0.1.0], [https://github.com/vivekvjnk/tiny-agent/issues])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/pty.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.15 foreign subdir-objects -Wall -Werror])
AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_INSTALL

# PTY support
AC_CHECK_HEADER([pty.h], [], [AC_MSG_ERROR([pty.h not found])])
AC_CHECK_LIB([util], [forkpty], [],
    [AC_MSG_ERROR([libutil with forkpty() not found])])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT
