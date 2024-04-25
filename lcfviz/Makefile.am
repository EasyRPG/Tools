direntdir = src/external/dirent_win

EXTRA_DIST = README.md \
	CMakeLists.txt \
	CMakeModules/ConfigureWindows.cmake \
	CMakeModules/FindICU.cmake \
	$(direntdir)

bin_PROGRAMS = lcfviz
lcfviz_SOURCES = \
	src/main.cpp \
	src/utils.cpp \
	src/utils.h \
	$(direntdir)/dirent_win.h
lcfviz_CXXFLAGS = \
	-std=c++17 \
	-I$(srcdir)/$(direntdir) \
	$(LCF_CFLAGS)
lcfviz_LDADD = \
	$(LCF_LIBS)
