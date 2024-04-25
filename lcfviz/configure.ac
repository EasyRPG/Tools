AC_INIT([lcfviz],[1.1],
	[https://github.com/EasyRPG/Tools/issues],[lcfviz],[https://easyrpg.org/])

AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_FILES([Makefile])

AC_PROG_CXX
PKG_CHECK_MODULES([LCF],[liblcf])

AC_OUTPUT
