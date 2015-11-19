XYZ2PNG
=======

XYZ2PNG is a small tool to convert RPG Maker 2000 and 2003 XYZ image file
format into PNG images.

XYZ2PNG is part of the EasyRPG Project.
More information is available at the project website:

https://easy-rpg.org/


Documentation
-------------

Documentation is available at the documentation wiki:

https://easy-rpg.org/wiki/


Requirements
------------

 * libpng
 * zlib


Daily builds
------------

Up to date binaries for assorted platforms are available at:

https://easy-rpg.org/jenkins/


Source code
-----------

XYZ2PNG development is hosted by GitHub, project files are available in Git
repositories.

https://github.com/EasyRPG/Tools


Building
--------

XYZ2PNG uses Autotools:

    ./bootstrap (only needed if using a git checkout)
    ./configure
    make
    make install (optionally)

You may tweak build parameters and environment variables, run
`./configure --help` for reference.


License
-------

XYZ2PNG is free software under the GNU General Public License Version 3. See
the file COPYING for details.
