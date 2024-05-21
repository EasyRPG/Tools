direntdir = src/external/dirent_win

EXTRA_DIST = AUTHORS.md README.md \
	CMakeLists.txt \
	CMakeModules/ConfigureWindows.cmake \
	CMakeModules/FindICU.cmake \
	$(direntdir)

bin_PROGRAMS = gencache
gencache_SOURCES = \
	src/main.cpp \
	$(direntdir)/dirent_win.h
gencache_CXXFLAGS = \
	-std=c++17 \
	-I$(srcdir)/$(direntdir) \
	$(ICU_CFLAGS) \
	$(NLOHMANNJSON_CFLAGS)
gencache_LDADD = \
	$(ICU_LIBS) \
	$(NLOHMANNJSON_LIBS)
