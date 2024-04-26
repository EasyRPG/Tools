# EasyRPG Tools

EasyRPG Tools is a suite of small file related applications to use and
convert RPG Maker 2000/2003 files.

EasyRPG Tools is part of the EasyRPG Project.
More information is available at the project website:

https://easyrpg.org/


## Tool details

These are the currently available tools:

 * LMU2PNG: renders LMU maps to PNG images with events as tiles support.

   Syntax: `lmu2png mapfile [Options]`

 * PNG2XYZ: converts PNG images into XYZ images. It supports wildcards.

   Syntax: `png2xyz file1 [... fileN]`

 * XYZ2PNG: converts XYZ images into PNG images. It supports wildcards.

   Syntax: `xyz2png file1 [... fileN]`

 * XYZCrush: makes smaller XYZ images. It supports wildcards.

   Syntax: `xyzcrush file1 [... fileN]`

 * GENCACHE: generates a JSON cache file of game directory contents.

   Syntax: `gencache [Options] [Directory]`

 * LcfTrans: extracts text out of LDB, LMT and LMU files and creates po files.

   Syntax: `lcftrans [Options] Directory [Encoding]`

 * LCFVIZ: Parses the map tree and creates a GraphViz (dot) graph.

   Syntax: `lcfviz [Options] [Directory]`

 * xyz-thumbnailer: displays thumbnails for XYZ files in your file manager
                    (currently Windows and Linux/GTK3/KDE5 only).

   Windows: Shell extension for Windows explorer, see included README.md for
            install instructions.

   Syntax (Linux/GTK3): `xyz-thumbnailer input output [size]`


## Daily builds


Up to date binaries are available at:

https://ci.easyrpg.org/


## Source code

EasyRPG Tools development is hosted by GitHub, project files are available in
Git repositories.

https://github.com/EasyRPG/Tools

### Note for Windows developers

*This only applies to git checkouts:*
The individual tool directories may share common configuration and external
libraries. These have been symlinked from the toplevel directory. To enable
these sysmlinks in git you need to have either a recent Windows version
(10, build 14972 or 11), on which you can enable `Developer Mode` OR need to
edit the user rights with Group policy editor to enable symlink creation.
Then, when using msysgit, there is a checkbox for symlink support, alternatively
the following command needs to be executed once to enable them globally:
`git config --global core.symlinks true`


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


License
-------

The EasyRPG Tools are free software under different licenses. Read the LICENSE
file available in the respective tool directories for details.
