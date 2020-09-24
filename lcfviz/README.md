LcfViz
======

LcfViz is a small tool that processes the map tree and the corresponding maps
to create a graph representation in "dot" format that can be consumed by the
popular graph rendering software GraphViz.

Map connections are determined by scanning for the Teleport event command.
Calls through Common Events are undetected, this would require an event
command interpreter. 

LcfViz is part of the EasyRPG Project.
More information is available at the project website:

https://easyrpg.org/


Documentation
-------------

Documentation is available at the documentation wiki:

https://easyrpg.org/wiki/


Requirements
------------

 * liblcf - https://github.com/EasyRPG/liblcf

Daily builds
------------

Up to date binaries for assorted platforms are available at:

https://ci.easyrpg.org/


Source code
-----------

LcfViz development is hosted by GitHub, project files are available in Git
repositories.

https://github.com/EasyRPG/Tools


Building
--------

LcfViz uses Autotools:

    ./bootstrap (only needed if using a git checkout)
    ./configure
    make
    make install (optionally)

You may tweak build parameters and environment variables, run
`./configure --help` for reference.


License
-------

LcfViz is free software under the MIT license. See the file COPYING for
details.
