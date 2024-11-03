AC_INIT([lmu2png],[1.2],
	[https://github.com/EasyRPG/Tools/issues],[lmu2png],[https://easyrpg.org/])

AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_FILES([Makefile])

AC_PROG_CXX
PKG_CHECK_MODULES([LCF],[liblcf])
PKG_CHECK_MODULES([ZLIB],[zlib])

PKG_CHECK_MODULES([FREEIMAGE],[FreeImage],,[
	AC_CHECK_HEADER([FreeImage.h],[freeimage_header=1],,[ ])
	AC_MSG_CHECKING([for FreeImage (legacy)])
	saved_ldflags="${LDFLAGS}"
	LDFLAGS="${LDFLAGS} -lfreeimage"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <FreeImage.h>],[FreeImage_GetVersion()])],[FREEIMAGE_LIBS="-lfreeimage"])
	LDFLAGS="${saved_ldflags}"
	AS_IF([test -z "$FREEIMAGE_LIBS" -o -z "$freeimage_header"],[AC_MSG_RESULT([no])
		AC_MSG_ERROR([Could not find FreeImage!])
	],[AC_MSG_RESULT([yes])])
])

AC_OUTPUT
