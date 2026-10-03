# frostbyte

This repo contains the engine module, desktop application, and server application for [frostbyte](https://github.com/frostbyte-engine).

# PROJECT STATE

This project is in early development stages!

In addition, I am frequently making drastic changes on my local machine before pushing to GitHub (I bounce back and forth between areas) so the state of the project rarely matches what is public.

[Issues](../../issues), however, usually closely match the project's real state.

# BUILDING
All dependencies besides openssl and ncurses/pdcurses are fetched and built by CMake.

## BUILDING (UNIX)
```bash
cmake -B build -S . -G Ninja # shouldn't have to be ninja, but it's proven to work for frostbyte
cmake --build build
```

## BUILDING (WINDOWS)
```bash
vcpkg install openssl:x64-windows
vcpkg install pdcurses:x64-windows
vcpkg install libgit2:x64-windows
```
```bash
cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DCURSES_INCLUDE_PATH=C:/path/to/vcpkg/installed/x64-windows/include ^
  -DCURSES_LIBRARY=C:/path/to/vcpkg/installed/x64-windows/lib/pdcurses.lib ^
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
If you know other methods of installing and configuring dependencies that don't involve vcpkg that you'd like to see listed here, reach out to me (if you have my contact great, otherwise you can make a [GitHub issue](../../issues)).

If you're unfamiliar with cmake, you can add -j8 to the end of the build command to speed things up: `cmake --build build -j8`.

~~ALSO SEE THE [workflow file](./.github/workflows/build-frostbyte-action.yml) FOR SYSTEM DEPENDENCY INFORMATION~~

# LUAU
frostbyte embeds [Luau](https://github.com/luau-lang/luau). See [luau_LICENSE.txt](luau_LICENSE.txt) for licensing information.
<br>
![](repoassets/luau.png)
