argparsedir = src/external/argparse
direntdir = src/external/dirent_win

EXTRA_DIST = README.md \
	CMakeLists.txt \
	CMakeModules/ConfigureWindows.cmake \
	CMakeModules/FindICU.cmake \
	$(argparsedir) $(direntdir)

bin_PROGRAMS = lcftrans
lcftrans_SOURCES = \
	src/entry.cpp \
	src/entry.h \
	src/main.cpp \
	src/translation.cpp \
	src/translation.h \
	src/types.h \
	src/utils.cpp \
	src/utils.h \
	$(argparsedir)/argparse.hpp \
	$(direntdir)/dirent_win.h
lcftrans_CXXFLAGS = \
	-std=c++17 \
	-I$(srcdir)/$(argparsedir) \
	-I$(srcdir)/$(direntdir) \
	$(LCF_CFLAGS)
lcftrans_LDADD = \
	$(LCF_LIBS)
