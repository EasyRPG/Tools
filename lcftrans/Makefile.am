bin_PROGRAMS = lcftrans
lcftrans_SOURCES = \
	src/dirent_win.h \
	src/entry.cpp \
	src/entry.h \
	src/main.cpp \
	src/translation.cpp \
	src/translation.h \
	src/types.h \
	src/utils.cpp \
	src/utils.h
lcftrans_CXXFLAGS = \
	-std=c++17 \
	$(LCF_CFLAGS)
lcftrans_LDADD = \
	$(LCF_LIBS)

EXTRA_DIST = README.md
