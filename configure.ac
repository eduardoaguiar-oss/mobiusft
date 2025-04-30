#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([mobiusft], [2.15], [aguiar@protonmail.ch])
AC_CONFIG_SRCDIR([src/mobius/bytearray.cc])
AM_INIT_AUTOMAKE([foreign -Wall -Werror dist-xz dist-zip no-dist-gzip subdir-objects])
AC_CONFIG_HEADERS([config.h])
AM_PROG_AR
LT_INIT
AC_CONFIG_MACRO_DIR([m4])

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Checking for programs, headers and libraries
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PKG_PROG_PKG_CONFIG
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_PROG_LIBTOOL
AX_CXX_COMPILE_STDCXX_17
AX_CXXFLAGS_WARN_ALL
AC_C_INLINE
AC_C_BIGENDIAN

AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h unistd.h])
AC_CHECK_HEADER_STDBOOL
AC_STRUCT_TM
AC_HEADER_TIME

AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset mkdir regcomp])
AC_CHECK_FUNCS([strchr strtoul strtoull])

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Checking datatypes
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
AC_TYPE_UID_T
AC_TYPE_OFF_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_CHECK_TYPES([ptrdiff_t])

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Checking for Python
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
AM_PATH_PYTHON(3.6)

AC_ARG_VAR([PYTHON_CONFIG], [Path to python-config])
AS_IF([test -z "$PYTHON_CONFIG"], [
  AC_PATH_PROGS([PYTHON_CONFIG],
                [python$PYTHON_VERSION-config python-config],
                [no],
                [`dirname $PYTHON`])
  AS_IF([test "$PYTHON_CONFIG" = no], [AC_MSG_ERROR([cannot find python-config for $PYTHON.])])
])
AC_MSG_CHECKING([for python include flags])
PY_CXXFLAGS=`$PYTHON_CONFIG --includes`
AC_MSG_RESULT([$PY_CXXFLAGS])

AC_MSG_CHECKING([for python libs])
PY_LIBS=`$PYTHON_CONFIG --libs`
AC_MSG_RESULT([$PY_LIBS])

pkgpyexecdir=$pyexecdir/mobius
pkgpythondir=$pyexecdir/mobius
AC_SUBST([PY_LIBS])
AC_SUBST([PY_CXXFLAGS])

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Check for CMake availability
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
AC_CHECK_PROG([CMAKE], [cmake], [cmake], [no])

AS_IF([test "x$CMAKE" = "xno"],
    [AC_MSG_ERROR([CMake is required to build this project. Please install CMake.])])

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Checking for pygtk
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
AC_MSG_CHECKING(for pygtk 3.0 or newer)
OUT=`$PYTHON -c "import gi; gi.require_version ('Gtk', '3.0')" 2>/dev/null`

if test $? -eq 0; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  AC_MSG_ERROR([PyGTK 3.0 or newer required. See https://www.gtk.org])
fi

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Checking for cairo
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
AC_MSG_CHECKING(for pycairo)
OUT=`$PYTHON -c "import gi; gi.require_foreign ('cairo')" 2>/dev/null`

if test $? -eq 0; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  AC_MSG_ERROR([PyCairo required. See https://cairographics.org/pycairo])
fi

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Checking for libtsk
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
AC_ARG_WITH(libtsk,
    [AS_HELP_STRING([--with-libtsk=DIR],[specify prefix directory for installed libtsk package.])],
    [with_libtsk=${withval}],
    [with_libtsk=yes])

if test "x${with_libtsk}" = "xno"; then
  have_libtsk=no

elif test "x${with_libtsk}" = "xyes"; then
  have_libtsk=yes

else
  have_libtsk=yes
  LIBTSK_DIR="${with_libtsk}"
  LIBTSK_CXXFLAGS="-I${with_libtsk}/include"

  if test -d ${with_libtsk}/lib64; then
    LIBTSK_LDFLAGS="-L${with_libtsk}/lib64"

  elif test -d ${with_libtsk}/lib; then
    LIBTSK_LDFLAGS="-L${with_libtsk}/lib"

  else
    AC_MSG_WARN([no /lib dir found at directory "${with_libtsk}"])
  fi
fi

# checks if libtsk is really available
if test "x${have_libtsk}" = "xyes"; then
  OLD_CPPFLAGS=$CPPFLAGS
  OLD_LDFLAGS=$LDFLAGS
  CPPFLAGS="$LIBTSK_CXXFLAGS $CPPFLAGS"
  LDFLAGS="$LIBTSK_LDFLAGS $LDFLAGS"

  AC_SEARCH_LIBS([tsk_version_get_str], [tsk], [have_libtsk=yes], [have_libtsk=no])
  AC_CHECK_HEADERS([tsk/libtsk.h], [], [have_libtsk=no])

  if test "x${have_libtsk}" = "xyes"; then
    MSG_SUPPORT_LIBTSK=yes
    AC_SUBST([LIBTSK_CXXFLAGS])
    AC_SUBST([LIBTSK_LDFLAGS])
  else
    MSG_SUPPORT_LIBTSK="no (libtsk not found. See www.sleuthkit.org)"
    CPPFLAGS=$OLD_CPPFLAGS
    LDFLAGS=$OLD_LDFLAGS
  fi
else
  MSG_SUPPORT_LIBTSK=no
fi

AM_CONDITIONAL([HAVE_LIBTSK], [test x${have_libtsk} = xyes])

if test "x${have_libtsk}" != "xyes"; then
  AC_MSG_ERROR([LIBTSK development files not found. See https://www.sleuthkit.org/sleuthkit])
fi

if test -n "$LIBTSK_DIR"; then
    AC_SUBST([LIBTSK_DIR])
    AC_MSG_NOTICE([Using libtsk directory: $LIBTSK_DIR])
fi

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Checking for libsmbclient
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
AC_ARG_WITH(libsmbclient,
    [AS_HELP_STRING([--with-libsmbclient=DIR],[specify prefix directory for installed libsmbclient package.])],
    [WITH_LIBSMBCLIENT=${withval}],
    [WITH_LIBSMBCLIENT=yes])

if test "x${WITH_LIBSMBCLIENT}" = "xno"; then
  MSG_SUPPORT_LIBSMBCLIENT="no"

elif test "x${WITH_LIBSMBCLIENT}" = "xyes"; then
  PKG_CHECK_MODULES([LIBSMBCLIENT], [smbclient], [have_libsmbclient=yes], [have_libsmbclient=no])

  if test "x${have_libsmbclient}" = "xyes"; then
    MSG_SUPPORT_LIBSMBCLIENT="yes"
    AC_DEFINE([HAVE_SMB_SUPPORT],[1],[Have SMB support])
  else
    MSG_SUPPORT_LIBSMBCLIENT="no (libsmbclient not found. See www.samba.org.)"
  fi

else
  have_libsmbclient="yes"
  MSG_SUPPORT_LIBSMBCLIENT="yes"
  LIBSMBCLIENT_CFLAGS="-I${WITH_LIBSMBCLIENT}/include"
  LIBSMBCLIENT_LIBS="-L${WITH_LIBSMBCLIENT}/lib -lsmbclient"
  AC_DEFINE([HAVE_SMB_SUPPORT],[1],[Have SMB support])
fi

AC_SUBST([LIBSMBCLIENT_CFLAGS])
AC_SUBST([LIBSMBCLIENT_LIBS])
AM_CONDITIONAL([HAVE_LIBSMBCLIENT], [test x${have_libsmbclient} = xyes])

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Generate files
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
AC_CONFIG_FILES([Makefile
                 mobius_bin.sh
                 src/Makefile
                 src/mobius/Makefile
                 src/mobius/io/Makefile
                 src/libmobius_core/Makefile
                 src/libmobius_framework/Makefile
                 src/libmobius_python/Makefile
                 src/python/Makefile
                 src/python/pymobius/Makefile
                 src/python/pymobius/ant/Makefile
                 src/python/pymobius/ant/evidence/Makefile
                 src/python/pymobius/ant/evidence/post/Makefile
                 src/python/pymobius/ant/evidence/ufdr/Makefile
                 src/python/pymobius/ant/evidence/vfs/Makefile
                 src/python/pymobius/ant/turing/Makefile
                 src/python/pymobius/app/Makefile
		 src/python/pymobius/app/chromium/Makefile
		 src/python/pymobius/app/edge/Makefile
		 src/python/pymobius/app/gecko/Makefile
		 src/python/pymobius/app/internet_explorer/Makefile
		 src/python/pymobius/app/itubego/Makefile
		 src/python/pymobius/app/skype/Makefile
		 src/python/pymobius/app/skype/skype/Makefile
		 src/python/pymobius/app/skype/app/Makefile
		 src/python/pymobius/app/skype/app_v14/Makefile
		 src/python/pymobius/app/utorrent/Makefile
                 src/python/pymobius/datasource/Makefile
                 src/python/pymobius/datasource/ufdr/Makefile
                 src/python/pymobius/operating_system/Makefile
                 src/python/pymobius/registry/Makefile
                 src/python/examples/Makefile
		 src/extensions/Makefile
                 src/tools/Makefile
                 data/Makefile
                 data/data/Makefile
                 data/icons/Makefile
                 data/icons/evidence/Makefile
                 data/icons/general/Makefile
                 include/Makefile
                 ])
AC_OUTPUT

# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# Print summary
# =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
AC_MSG_NOTICE([
   mobius::filesystem::tsk......: $MSG_SUPPORT_LIBTSK
   mobius::io::smb..............: $MSG_SUPPORT_LIBSMBCLIENT
]);
