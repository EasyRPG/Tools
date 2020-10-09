bin_PROGRAMS = gencache
gencache_SOURCES = \
	src/main.cpp
gencache_CXXFLAGS = \
	-std=c++11 \
	$(ICU_CFLAGS) \
	$(NLOHMANNJSON_CFLAGS)
gencache_LDADD = \
	$(ICU_LIBS) \
	$(NLOHMANNJSON_LIBS)

EXTRA_DIST = AUTHORS.md README.md
