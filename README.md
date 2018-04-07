# XLEngine

TODO
====
* game plugin fix-ups
* have cmake place images, as and other things into the build directory


Dependancies
============
* DevIL (libdevil-dev) Cross-platform image loading and manipulation toolkit
* enet (libenet-dev) Thin network communication layer on top of UDP
* GLEW (libglew-dev) OpenGL Extension Wrangler Library
* SDL2 (libsdl2-dev) Cross-platform low-level I/O access
* cmake


Building
========
We use Cmake as a cross-platform build system.

```bash
git checkout https://github.com/Mindwerks/XLEngine.git;
mkdir build; cd build; cmake ../XLEngine; make -j4
```

Changelog
=========

0.2.0
-----
* dos2unix all files
* PlatformDef magic with macros
* bare minimum fixes to compile on linux with gcc
* added cross-platform cmake build system
* re-added angelscript external dep


0.1.0 Initial Release
---------------------
* LuciusDXL code drop from 2011-07-12 09:24:47 +0200 (Tue, 12 Jul 2011)
* LuciusDXL licensed code under the Expat License (aka MIT License)
* Original LuciusDXL release will be included in 0.1.0 tagged release
* Added README.md
* Removed 3rd-party libs (will add back as necessary)
* Removed binaries and MSVC related files


