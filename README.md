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


Runtime Configuration
=====================
Game data paths and render settings are specified in XLEngine.conf. On non-
Windows systems, it will look for this in the XDG standard directory:
```
$XDG_CONFIG_HOME/XLEngine/XLEngine.conf
```
or, if `XDG_CONFIG_HOME` is unset or empty:
```
$HOME/.config/XLEngine/XLEngine.conf
```

On all systems, it will then search for XLEngine.conf in the process's working
directory and use the settings there as overrides.

The available settings are:
```
[general]
width = # window/screen width, 320 minimum
height = # window/screen height, 200 minimum
fullscreen = # true or false
vsync = # true or false
emulate-low-res = # true to render at 320x200 and stretch to fit, or false
renderer = # opengl, soft32 (true-color software), soft8 (paletted software)

[DaggerXL]
data-path = # Full path to game data, e.g. C:\DAGGER\ARENA2

[DarkXL]
data-path = # ...

[BloodXL]
data-path = # ...

[OutlawsXL]
data-path = # ...
```

Currently the game must be run from where it can find the DarkXL/, DaggerXL/,
etc. directories, where the AngelScript (*.as) and image (*.png) files are for
the respective games, and the fonts/ directory with the necessary font files.


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


