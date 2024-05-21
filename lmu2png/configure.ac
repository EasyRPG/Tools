AC_INIT([lmu2png],[2.0],
	[https://github.com/EasyRPG/Tools/issues],[lmu2png],[https://easyrpg.org/])

AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_FILES([Makefile])

AC_PROG_CXX
PKG_CHECK_MODULES([LCF],[liblcf])
PKG_CHECK_MODULES([SDL2_IMAGE],[SDL2_image])
PKG_CHECK_MODULES([ZLIB],[zlib])

AC_OUTPUT
