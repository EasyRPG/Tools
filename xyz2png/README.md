# XYZ2PNG

XYZ2PNG is a small tool to convert RPG Maker 2000 and 2003 XYZ image file
format into PNG images.

XYZ2PNG is part of the EasyRPG Project. More information is available
at the project website: https://easyrpg.org/


## Documentation

Documentation is available at the documentation wiki: https://wiki.easyrpg.org


## Requirements

- [zlib] for XYZ file compressed structure reading. (required)
- [libpng] for PNG writing support. (required)


## Daily builds

Up to date binaries for various systems are available at https://ci.easyrpg.org


## Source code

XYZ2PNG development is hosted by GitHub, project files are available
in this git repository:

https://github.com/EasyRPG/Tools


## Building

XYZ2PNG uses Autotools:

    ./bootstrap (only needed if using a git checkout)
    ./configure
    make
    make install (optionally)

You may tweak build parameters and environment variables, run
`./configure --help` for reference.


## License

XYZ2PNG is Free/Libre Open Source Software, released under the MIT License.
See the file [COPYING] for copying conditions.


[zlib]: https://zlib.net
[libpng]: http://libpng.org/pub/png/libpng.html
[COPYING]: COPYING
