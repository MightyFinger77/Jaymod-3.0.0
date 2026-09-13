# Build

This tree’s CMake target is **64-bit ET: Legacy** modules. Stock ET 2.60b `etded` will not load these DLLs.

## Requirements (Windows)

- CMake 3.16+
- A 64-bit toolchain: Visual Studio 2019+ **or** mingw-w64 (WinLibs UCRT)

```
cmake -S . -B build
cmake --build build --config Release
```

MSVC: add `-A x64` on configure.

Outputs (MinGW: `build/`, MSVC: `build/Release/`):

| File | Role |
| --- | --- |
| `qagame_mp_x64.dll` | Server. Leave loose in the `jaymod` folder. |
| `qagame_mp_x86_64.dll` | Copy of the same server DLL |
| `cgame_mp_x64.dll` | 64-bit client game — pack into the pk3 |
| `ui_mp_x64.dll` | 64-bit client UI — pack into the pk3 |

Also copy `*_x86_64.dll` names for older ETL loaders.

`tools/pack-release.ps1` also builds **32-bit** `cgame_mp_x86.dll` / `ui_mp_x86.dll` with the i686 MinGW in `tools/mingw32` (`build-x86/`) and packs those into the pk3. Vanilla ET / 32-bit ETL need those, not the leftover 2.3.0 binaries. Do not put `qagame` in the pk3.

`FEATURE_LUA` is on for `qagame`. Lua 5.1 is compiled in from `src/lua/`.

## Linux (same CMake)

Produces `qagame.mp.x86_64`, `cgame.mp.x86_64`, `ui.mp.x86_64`. Omni-bot on Linux 64-bit is `omnibot_et.x86_64.so`.

## What not to use

- The old Python 2 / makefile pack (`pak.defs`) is still in the tree for reference.
- RNGesus 2.3.0 docs mentioned `JAYMOD_M32=ON` for stock 32-bit ET. That flag is **not** this CMake. This fork is the 64-bit etlded line.
- MinGW is what this machine used in development (`-fpermissive`, static libgcc). MSVC needs the `.def` files under `src/game`, `src/cgame`, `src/ui`.

## Version stamp

`cmake/project.h.in` sets `JAYMOD_version` / `jaymod-3.1.0` / `jaymod-3.1.0.pk3`. Bump those plus `project/info.db` if you cut a new point release.

`tools/pack-release.ps1` rebuilds 32-bit `cgame`/`ui` and stamps them `Jaymod 3.1.0`. A leftover 2.3.0 / 3.0.0 x86 module aborts in `CG_Init` on 32-bit ETL.
