AC_INIT(gtkparasite, 0.2.0, chipx86@chipx86.com)
AC_PREREQ(2.50)
AC_CONFIG_SRCDIR(config.h.in)
AC_COPYRIGHT([Copyright 2008-2012 Christian Hammond])

dnl ################################################################
dnl # libtool versioning
dnl ################################################################
dnl #
dnl # +1 :  0 : +1   == new interface that does not break old one.
dnl # +1 :  0 :  ?   == removed an interface. Breaks old apps.
dnl #  ? : +1 :  ?   == internal changes that doesn't break anything.
dnl #
dnl # CURRENT : REVISION : AGE
dnl #
LT_CURRENT=0
LT_REVISION=0
LT_AGE=0

AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)


dnl ################################################################
dnl # Initialize automake
dnl ################################################################
VERSION=0.2.0
PACKAGE=gtkparasite

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)


dnl ################################################################
dnl # Specify a header configuration file
dnl ################################################################
AM_CONFIG_HEADER(config.h)


dnl ################################################################
dnl # Check for some standard stuff.
dnl ################################################################
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_INSTALL
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

AC_C_CONST
AC_TYPE_SIZE_T
AC_EXEEXT

AC_MSG_CHECKING([which gtk+ version to compile against])
AC_ARG_WITH([gtk],
    [AS_HELP_STRING([--with-gtk=2.0|3.0],[which gtk+ version to compile against (default: 2.0)])],
    [case "$with_gtk" in
        2.0|3.0) GTK_API_VERSION=$with_gtk ;;
        *) AC_MSG_ERROR([invalid gtk version specified]) ;;
        esac],
    [GTK_API_VERSION=2.0])
AC_MSG_RESULT([$GTK_API_VERSION])
AC_SUBST(GTK_API_VERSION)

AM_CONDITIONAL(GTK3, [test $GTK_API_VERSION = 3.0])

GDK_BACKENDS=

case "$GTK_API_VERSION" in
3.0)
	GTK_REQUIRED=3.0.0
	GDK_BACKENDS=`pkg-config --variable=targets gtk+-$GTK_API_VERSION`
    ;;
2.0)
	GTK_REQUIRED=2.0.0
	GDK_BACKENDS=`pkg-config --variable=target gtk+-$GTK_API_VERSION`
    ;;
esac
PKG_CHECK_MODULES(GTK, [gtk+-$GTK_API_VERSION >= $GTK_REQUIRED])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

case "$GDK_BACKENDS" in *x11*) have_x11=true ;; *) have_x11=false ;; esac

AM_CONDITIONAL(HAVE_X11, [test x$have_x11 = xtrue])

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
    case "$GDK_BACKENDS" in
    	*quartz*) PYTHON_SHARED_LIB="libpython${PYTHON_VERSION}.dylib" ;;
	*)        PYTHON_SHARED_LIB="libpython${PYTHON_VERSION}.so" ;;
    esac
    AC_SUBST(PYTHON_SHARED_LIB)

    case "$GTK_API_VERSION" in
    3.0)
        PKG_CHECK_MODULES(PYGTK,
                          [pygobject-3.0 >= 3.0.0],
                          [
                            has_pygtk=yes
                            AC_DEFINE(ENABLE_PYTHON, [1], [Enable Python support])
                            AC_DEFINE(USE_GOBJECT_INTROSPECTION, [1], [Use GIR support for GTK+3])
                          ],
                          [has_pygtk=no])
    ;;
    2.0)
        PKG_CHECK_MODULES(PYGTK,
                          [pygobject-2.0 >= 2.0.0],
                          [
                            has_pygtk=yes
                            AC_DEFINE(ENABLE_PYTHON, [1], [Enable Python support])
                          ],
                          [has_pygtk=no])
    ;;
    esac
], [has_python=no])

AM_CONDITIONAL(ENABLE_PYTHON, [test x"$has_pygtk" = x"yes"])

AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_LIBS)
AC_SUBST(PYGTK_CFLAGS)
AC_SUBST(PYGTK_LIBS)


dnl # Use wall if we have GCC
if test "x$GCC" = "xyes"; then
   CFLAGS="$CFLAGS -Wall"
fi

AC_SUBST(CFLAGS)

dnl ################################################################
dnl # Output the Makefiles
dnl ################################################################
AC_CONFIG_FILES([
Makefile
src/Makefile
])

AC_OUTPUT

echo
echo $PACKAGE v$VERSION
echo
echo "GTK+ version:   $GTK_API_VERSION"
echo "Python support: $has_pygtk"
echo "GDK backends:   $GDK_BACKENDS"

echo
echo "Now type make to compile"
echo "Then su to root and type: make install"
echo
