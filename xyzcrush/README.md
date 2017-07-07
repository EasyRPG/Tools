XYZCrush
========

XYZCrush is a small tool to recompress RPG Maker 2000 and 2003 XYZ image file
format into smaller files.

XYZCrush part of the EasyRPG Project.
More information is available at the project website:

https://easyrpg.org/


Documentation
-------------

Documentation is available at the documentation wiki:

https://wiki.easyrpg.org/


Requirements
------------

 * zlib


Daily builds
------------

Up to date binaries for assorted platforms are available at:

https://ci.easyrpg.org/


Source code
-----------

XYZCrush development is hosted by GitHub, project files are available in Git
repositories.

https://github.com/EasyRPG/Tools


Building
--------

XYZCrush uses Autotools:

    ./bootstrap (only needed if using a git checkout)
    ./configure
    make
    make install (optionally)

You may tweak build parameters and environment variables, run
`./configure --help` for reference.


License
-------

XYZCrush is free software under the GNU General Public License Version 3.
See the file COPYING for details.

XYZCrush includes a copy of the Zopfli source code.
Zopfli is free software under the Apache 2.0 license.
