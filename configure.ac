AC_INIT(options_to_options,0.9-git, ola@nlogic.no)
AM_INIT_AUTOMAKE([-Wall -Wno-extra-portability -Werror foreign])
LT_INIT
AC_CONFIG_MACRO_DIR([m4])
# Use C++ language
# AC_PROG_AR
AC_PROG_CXX

# AC_LANG([C++])

# On FreeBSD (and probably some others), clang++ does not meet an autoconf
# assumption in identifying libtool configuration regarding shared library:
# the configure script will execute "$CC -shared $CFLAGS/$CXXFLAGS -v" and
# expect the output contains -Lxxx or -Ryyy.  This is the case for g++, but
# not for clang++, and, as a result, it will cause various errors in linking
# programs or running them with a shared object (such as some of our python
# scripts).
# To work around this problem we define a temporary variable
# "CXX_LIBTOOL_LDFLAGS".  It's expected to be defined as, e.g, "-L/usr/lib"
# to temporarily fake the output so that it will be compatible with that of
# g++.
CFLAGS_SAVED=$CFLAGS
CXXFLAGS_SAVED=$CXXFLAGS
CFLAGS="$CFLAGS $CXX_LIBTOOL_LDFLAGS"
CXXFLAGS="$CXXFLAGS $CXX_LIBTOOL_LDFLAGS"
LT_INIT
CFLAGS=$CFLAGS_SAVED
CXXFLAGS=$CXXFLAGS_SAVED

# Use C++ language
AC_LANG([C++])

# Identify the compiler: this check must be after AC_PROG_CXX and AC_LANG.
AM_CONDITIONAL(USE_GXX, test "X${GXX}" = "Xyes")
AC_CHECK_DECL([__SUNPRO_CC], [SUNCXX="yes"], [SUNCXX="no"])
AC_CHECK_DECL([__clang__], [CLANGPP="yes"], [CLANGPP="no"])
AM_CONDITIONAL(USE_CLANGPP, test "X${CLANGPP}" = "Xyes")



AC_CONFIG_FILES([
 Makefile
])
AC_OUTPUT
