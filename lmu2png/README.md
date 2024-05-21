# LMU2PNG

LMU2PNG is a tool to render RPG Maker 2000 and 2003 map data into PNG images.

LMU2PNG is part of the EasyRPG Project.
More information is available at the project website:

https://easyrpg.org/


## Documentation

Documentation is available at the documentation wiki:

https://wiki.easyrpg.org/


## Requirements

 * liblcf - https://github.com/EasyRPG/liblcf
 * zlib
 * SDL2_image (enable support for at least png images)
 * wxWidgets (optional, GUI version)


## Daily builds

Up to date binaries for assorted platforms are available at:

https://ci.easyrpg.org/


## Source code

LMU2PNG development is hosted by GitHub, project files are available in Git
repositories.

https://github.com/EasyRPG/Tools


## Building

### Autotools:

```shell
./bootstrap # (only needed if using a git checkout)
./configure
make
make install # (optionally)
```

You may tweak build parameters and environment variables, run
`./configure --help` for reference.

### CMake

```shell
cmake -B builddir
cmake --build builddir
cmake --install builddir # (optionally)
```


## License

LMU2PNG is free software under the GNU General Public License Version 3. See
the file COPYING for details.
