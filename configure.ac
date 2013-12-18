AC_PREREQ([2.64])
AC_INIT([gtkparasite],[0.3.0],[chipx86@chipx86.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/window.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_COPYRIGHT([Copyright 2008-2012 Christian Hammond])

AM_INIT_AUTOMAKE([1.11.1 -Wall dist-xz no-define no-dist-gzip])
AM_SILENT_RULES([yes])


AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_AR
LT_INIT

GTK_REQUIRED=3.10.0
PKG_CHECK_MODULES(GTK, [gtk+-3.0 >= $GTK_REQUIRED gtksourceview-3.0])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AM_PATH_PYTHON([2.4], [
    AC_PATH_PROG(PYTHON_CONFIG, python$PYTHON_VERSION-config)
    if test x"$PYTHON_CONFIG" == x""; then
        AC_PATH_PROG(PYTHON_CONFIG, python-config)
    fi
    if test x"$PYTHON_CONFIG" != x""; then
        PYTHON_CFLAGS=`$PYTHON_CONFIG --includes`
        PYTHON_LIBS=`$PYTHON_CONFIG --libs`
    else
        PYTHON_CFLAGS=`$PYTHON $srcdir/python-config.py --includes`
        PYTHON_LIBS=`$PYTHON $srcdir/python-config.py --libs`
    fi

    # Kind of a hacky way to get this, but that's alright..
    if test x"$GDK_BACKEND" == x"quartz"; then
        PYTHON_SHARED_LIB="libpython${PYTHON_VERSION}.dylib"
    else
        PYTHON_SHARED_LIB="libpython${PYTHON_VERSION}.so"
    fi
    AC_SUBST(PYTHON_SHARED_LIB)

    PKG_CHECK_MODULES(PYGTK,
                      [pygobject-3.0 >= 3.0.0],
                      [
                       has_pygtk=yes
                       AC_DEFINE(ENABLE_PYTHON, [1], [Enable Python support])
                      ],
                      [has_pygtk=no])
], [has_python=no])

AM_CONDITIONAL(ENABLE_PYTHON, [test x"$has_pygtk" = x"yes"])

AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_LIBS)
AC_SUBST(PYGTK_CFLAGS)
AC_SUBST(PYGTK_LIBS)

AC_CONFIG_FILES([
Makefile
src/Makefile
])

AC_OUTPUT

AC_MSG_NOTICE([

$PACKAGE_NAME v$PACKAGE_VERSION configure summary
====================================

Python support: $has_pygtk
])
