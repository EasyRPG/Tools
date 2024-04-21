GENCACHE
========

GENCACHE is a small tool to generate a file listing cache used by the
EasyRPG Player on some platforms.

GENCACHE is part of the EasyRPG Project.
More information is available at the project website:

https://easyrpg.org/


Documentation
-------------

Documentation is available at the documentation wiki:

https://wiki.easyrpg.org/


Requirements
------------

 * ICU
 * nlohmann json


Daily builds
------------

Up to date binaries for assorted platforms are available at:

https://ci.easyrpg.org/


Source code
-----------

GENCACHE development is hosted by GitHub, project files are available in a git
repository.

https://github.com/EasyRPG/Tools


Building
--------

GENCACHE uses Autotools:

    ./bootstrap (only needed if using a git checkout)
    ./configure
    make
    make install (optionally)

You may tweak build parameters and environment variables, run
`./configure --help` for reference.


License
-------

GENCACHE is free software under the ISC license. See the file COPYING for
details.

It includes "Dirent interface for Microsoft Visual Studio" by Toni Ronkko,
available under MIT license at: https://github.com/tronkko/dirent
