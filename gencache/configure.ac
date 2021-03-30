AC_INIT([gencache],[1.1],[https://github.com/EasyRPG/Tools/issues],[gencache],[https://easyrpg.org/])

AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall dist-xz])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_FILES([Makefile])
AC_LANG([C++])
AC_PROG_CXXCPP
AC_PROG_CXX

PKG_CHECK_MODULES([ICU], [icu-i18n])
PKG_CHECK_MODULES([NLOHMANNJSON], [nlohmann_json],,[
	AC_CHECK_HEADER([nlohmann/json.hpp],,[
		AC_MSG_ERROR([Could not find 'nlohmann_json' package! Consider installing version 3.9.0 or newer.])
	],[ ])
])

AC_OUTPUT
