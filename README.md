# frostbyte

This repo contains the engine module for [frostbyte](https://github.com/frostbyte-engine).

# PROJECT STATE

This project is in early development stages! This is why there is no Windows support!

In addition, I am frequently making drastic changes on my local machine before pushing to GitHub (I bounce back and forth between areas) so the state of the project rarely matches what is public.

[Issues](../../issues), however, usually closely match the project's real state.

# BUILDING
NOTE: frostbyte CURRENTLY does _not_ have a process for building neither for or on Windows. It is likely possible to cross compile via mingw, but that would require manual steps.

First clone the repo and initialize submodules:
```bash
git clone https://github.com/frostbyte-engine/frostbyte.git
cd frostbyte
git submodule update --init --recursive
```

frostbyte uses [mate.h](https://github.com/TomasBorquez/mate.h/) for its core build system, along with some python files to build the dependencies.
<br>
To build dependencies, first ensure you have their respective dependencies installed. You can usually find this on each project page, but here is a command you can run on Ubuntu for reference:
```bash
sudo apt-get install cmake build-essential git \
    libasound2-dev \
    libx11-dev \
    libxrandr-dev \
    libxi-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libxcursor-dev \
    libxinerama-dev \
    libwayland-dev \
    libxkbcommon-dev \
    libpsl-dev
```
Then just run each script inside the dependencies folder with python:
```bash
cd dependencies

python3 ./build_curl.py & python3 ./build_luau.py

cd ..
```

then, compile and run mate.c (note that I target gcc, so clang and msvc may or may not be supported):
```bash
gcc -o mate mate.c
./mate
```

You should now have a `build/libfrostbyte.a` file.
<br>
That's it! To build again, simply run `./mate` just like before and it will detect any changes made and recompile only what's needed.

# LUAU
frostbyte embeds [Luau](https://github.com/luau-lang/luau). See [luau_LICENSE.txt](luau_LICENSE.txt) for licensing information.
<br>
![](repoassets/luau.png)
