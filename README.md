EasyRPG Tools
=============

EasyRPG Tools is a suite of small file related applications to use and
convert RPG Maker 2000/2003 files.

EasyRPG Tools is part of the EasyRPG Project.
More information is available at the project website:

https://easyrpg.org/


Tool details
------------

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

 * xyz-thumbnailer: displays thumbnails for XYZ files in your file manager
                    (currently Windows and Linux/GTK3/KDE5 only).

   Windows: Shell extension for Windows explorer, see included README.md for
            install instructions.

   Syntax (Linux/GTK3): `xyz-thumbnailer input output [size]`


Daily builds
------------

Up to date binaries are available at:

https://ci.easyrpg.org/


Source code
-----------

EasyRPG Tools development is hosted by GitHub, project files are available in
Git repositories.

https://github.com/EasyRPG/Tools


License
-------

The EasyRPG Tools are free software under different licenses. Read the LICENSE
file available in the respective tool directories for details.
