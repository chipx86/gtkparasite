AC_INIT(gtkparasite, 0.1.0, chipx86@chipx86.com)
AC_PREREQ(2.50)
AC_CONFIG_SRCDIR(config.h.in)
AC_COPYRIGHT([Copyright 2008 Christian Hammond])

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
VERSION=0.1.0
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
AM_PROG_LIBTOOL

AC_C_CONST
AC_TYPE_SIZE_T
AC_EXEEXT


PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.4.0])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)


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
echo "Now type make to compile"
echo "Then su to root and type: make install"
echo
