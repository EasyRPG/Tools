# XYZCrush

XYZCrush is a small tool to recompress RPG Maker 2000 and 2003 XYZ image file
format into smaller files.

XYZCrush is part of the EasyRPG Project. More information is available
at the project website: https://easyrpg.org/


## Documentation

Documentation is available at the documentation wiki: https://wiki.easyrpg.org


## Requirements

- [zlib] for XYZ file compressed structure reading. (required)


## Daily builds

Up to date binaries for various systems are available at https://ci.easyrpg.org


## Source code

XYZCrush development is hosted by GitHub, project files are available
in this git repository:

https://github.com/EasyRPG/Tools


## Building

XYZCrush uses Autotools:

    ./bootstrap (only needed if using a git checkout)
    ./configure
    make
    make install (optionally)

You may tweak build parameters and environment variables, run
`./configure --help` for reference.


## License

XYZCrush is Free/Libre Open Source Software, released under the MIT License.
See the file [COPYING] for copying conditions.


### 3rd party software

XYZCrush includes code of the following 3rd party software:

- [zopfli] under the Apache 2.0 license.

See the source code comment headers for license details.


[zlib]: https://zlib.net
[COPYING]: COPYING
[zopfli]: https://github.com/google/zopfli
