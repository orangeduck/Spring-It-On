Spring-It-On: The Game Developer's Spring-Roll-Call
===================================================

This repo contains the source code for all the demos from [this article](http://theorangeduck.com/page/spring-roll-call). 

It uses [raylib](https://www.raylib.com/) or more specifically [raygui](https://github.com/raysan5/raygui) so if you have that installed it should be easy to play around and try them out.

## Install dependencies

* Install `raylib` using your package manager of choice.
* Download [raygui release](https://github.com/raysan5/raygui/releases) and copy `raygui.h` to this directory.

## CMake build

```bash
mkdir build
cd build

# Release
cmake -DCMAKE_BUILD_TYPE=Release ..

# Debug
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build
make -j

# Run apps
bin/damper
bin/smoothing
# and so forth
```
