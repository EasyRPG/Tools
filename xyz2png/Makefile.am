EXTRA_DIST = README.md \
	CMakeLists.txt CMakeModules/ConfigureWindows.cmake

bin_PROGRAMS = xyz2png
xyz2png_SOURCES = src/xyz2png.cpp
xyz2png_CXXFLAGS = \
	$(PNG_CFLAGS) \
	$(ZLIB_CFLAGS)
xyz2png_LDADD = \
	$(PNG_LIBS) \
	$(ZLIB_LIBS)
