argparsedir = src/external/argparse

EXTRA_DIST = README.md \
	CMakeLists.txt \
	CMakeModules/ConfigureWindows.cmake \
	CMakeModules/FindFreeImage.cmake \
	CMakeModules/FindICU.cmake \
	$(argparsedir)

bin_PROGRAMS = lmu2png
lmu2png_SOURCES = \
	src/main.h \
	src/main.cpp \
	src/chipset.h \
	src/chipset.cpp \
	src/xyzplugin.h \
	src/xyzplugin.cpp \
	$(argparsedir)/argparse.hpp
lmu2png_CXXFLAGS = \
	-std=c++17 \
	-I$(srcdir)/$(argparsedir) \
	$(LCF_CFLAGS) \
	$(FREEIMAGE_CFLAGS) \
	$(ZLIB_CFLAGS)
lmu2png_LDADD = \
	$(LCF_LIBS) \
	$(FREEIMAGE_LIBS) \
	$(ZLIB_LIBS)
