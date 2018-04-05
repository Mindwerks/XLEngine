# XLEngine

TODO
====
* fix world/ so it compiles with gcc -c *.cpp
* fix scriptsystem/ so it compiles with gcc -c *.cpp


Dependancies
============
* DevIL (libdevil-dev) Cross-platform image loading and manipulation toolkit
* enet (libenet-dev) Thin network communication layer on top of UDP
* angelscript (not in Debian/Ubuntu) Flexible cross-platform scripting library

Changelog
=========

0.2.0
-----
* dos2unix all files
* PlatformDef magic with macros
* bare minimum fixes to compile on linux with gcc


0.1.0 Initial Release
---------------------
* LuciusDXL code drop from 2011-07-12 09:24:47 +0200 (Tue, 12 Jul 2011)
* LuciusDXL licensed code under the Expat License (aka MIT License)
* Original LuciusDXL release will be included in 0.1.0 tagged release
* Added README.md
* Removed 3rd-party libs (will add back as necessary)
* Removed binaries and MSVC related files


linking it all together
=======================

gcc -c *.cpp (repeat for all directories)
cd linux
reset; gcc -o XLEngine main.o -lGL -lX11 ../*.o -lm ../fileformats/*.o ../math/*.o ../memory/*.o ../movieplayback/*.o ../networking/*.o ../os/linux/*.o ../os/*.o ../plugin_framework/*.o -ldl ../procedural/*.o ../render/linux/*.o ../render/triangleRasterizer/*.o  ../render/*.o ../ui/*.o ../world/*.o -lstdc++ -lGLEW -lIL -lenet
