LCF2XML
=======

LCF2XML is a small tool to convert RPG Maker 2000 and 2003 data format
(LMU, LMT, LDB, LSD, ...) to XML and vice-versa.

LCF2XML is part of the EasyRPG Project.
More information is available at the project website:

https://easy-rpg.org/


Documentation
-------------

Documentation is available at the documentation wiki:

https://easy-rpg.org/wiki/


Requirements
------------

* liblcf (with XML support) - https://github.com/EasyRPG/liblcf


Daily builds
------------

Up to date binaries for assorted platforms are available at:

https://easy-rpg.org/jenkins/


Source code
-----------

LCF2XML development is hosted by GitHub, project files are available in Git
repositories.

https://github.com/EasyRPG/Tools


Building
--------

LCF2XML uses Autotools:

    ./bootstrap (only needed if using a git checkout)
    ./configure
    make
    make install (optionally)

You may tweak build parameters and environment variables, run
`./configure --help` for reference.


License
-------

LCF2XML is free software under the MIT license. See the file COPYING for
details.
