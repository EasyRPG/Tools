AC_INIT([easyrpg-tools],[1.0],[https://github.com/EasyRPG/Tools/issues],[easyrpg-tools],[https://easyrpg.org/])

AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])
AM_SILENT_RULES([yes])

# helper func
AC_DEFUN([EASYRPG_TOOL_ENABLE],[
	AC_ARG_ENABLE([$1],
		AS_HELP_STRING([--disable-$1],
			[disable $1 tool @<:@default=enabled@:>@]))
	AS_IF([test "x$enable_$1" != xno],[
		enable_$1=yes
		AC_CONFIG_SUBDIRS([$1])
	],[])
	AM_CONDITIONAL(ENABLE_[]translit([[$1]], [a-z], [A-Z]), [test "x$enable_$1" = xyes])
])

EASYRPG_TOOL_ENABLE([lcf2xml])
EASYRPG_TOOL_ENABLE([lmu2png])
EASYRPG_TOOL_ENABLE([png2xyz])
EASYRPG_TOOL_ENABLE([xyz2png])
EASYRPG_TOOL_ENABLE([xyzcrush])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

echo ""
echo "Summary:"
echo "========"
echo ""
echo "  prefix:  $prefix"
echo ""
echo "Converters:"
echo "  lcf2xml: $enable_lcf2xml"
echo "  lmu2png: $enable_lmu2png"
echo "  png2xyz: $enable_png2xyz"
echo "  xyz2png: $enable_xyz2png"
echo ""
echo "Other:"
echo "  xyzcrush: $enable_xyzcrush"
