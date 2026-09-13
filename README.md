# Jaymod 3.0.0

Fork of [RNGesus Jaymod 2.3.0](https://github.com/RngesusSolutions/jaymod2.2.0) for **ET: Legacy 2.85+ 64-bit `etlded`**. Gameplay is still Jaymod. 2.3.0 fixed FPS-dependent timing. This tree adds a native Lua host, 64-bit modules, and a working Omni-bot 0.93 load on Win64.

Lineage: official **Jaymod 2.2.0** (Jaybird, 2011) → **2.3.0** (RNGesus, 2026) → **3.0.0** (this fork).

| | |
| --- | --- |
| **Mod version** | 3.0.0 |
| **Dedicated server** | 64-bit ET: Legacy `etlded` (protocol 84) |
| **Clients** | Vanilla ET 2.60b / 32-bit ETL **and** 64-bit ETL, same pk3 |
| **Lua** | Lua 5.1 in `qagame` (`et_*` hooks, `et.*` functions) |
| **Omni-bot** | 0.8 game interface + Omni-bot **0.93** `omnibot_et_x64.dll` |
| **This archive’s binaries** | **Windows x64 only** |
| **License** | Apache 2.0 + original id Software terms |

## What you get

Release zip: `jaymod-3.0.0-etlded-server.zip`

```
jaymod/jaymod-3.0.0.pk3          clients download this
jaymod/qagame_mp_x64.dll         required by 64-bit etlded
jaymod/qagame_mp_x86_64.dll      same binary, older ETL filename
jaymod/SERVER.txt                short install note
```

The pk3 ships both client bitnesses:

| Files | Who loads them |
| --- | --- |
| `cgame_mp_x86.dll` / `ui_mp_x86.dll` | ET 2.60b and 32-bit ETL |
| `cgame_mp_x64.dll` / `ui_mp_x64.dll` | 64-bit ETL (`*_x86_64` names included) |

`qagame` stays **outside** the pk3. Do not pack it in.

**Not in this zip:** Linux `.so` modules (`qagame.mp.x86_64`, `cgame.mp.x86_64`, `ui.mp.x86_64`). CMake can build those on a 64-bit Linux box; this archive was produced on Windows.

## Server install

1. Run **64-bit** `etlded.exe` (ET: Legacy 2.85 or newer). Stock 32-bit `etded` will not load these DLLs.
2. Copy `jaymod-3.0.0.pk3` and `qagame_mp_x64.dll` into the `jaymod` folder the dedicated actually searches (`.\jaymod` or the ETL homepath `jaymod` folder — check `Sys_LoadDll` in the log).
3. Remove older `jaymod-2.2.0.pk3` / `jaymod-2.3.0.pk3` on that server so clients fetch 3.0.0.
4. Keep mapscripts, `shrubbot.cfg`, and `user.db`.
5. Start with `+set fs_game jaymod`.
6. Leave `g_requireClientVersion 0` if anyone still has a 2.2.0 / 2.3.0 client pk3.

A 32-bit client never loads the x64 `cgame`/`ui`. They join the same 64-bit dedicated.

## Omni-bot

Use ETL’s **64-bit** Omni-bot tree (0.93), not an old 32-bit `omnibot_et.dll`.

Copy the whole `legacy/omni-bot` folder (`omnibot_et_x64.dll`, `et/`, `global_scripts/`) and point at it:

```
set omnibot_enable "1"
set omnibot_path "C:/Servers/YourServer/omni-bot"
```

A good load prints `OMNIBOT: load '...omnibot_et_x64.dll': success` then `initialization: success`. After that, bots should add on `MaxBots` / `pfnUpdate` (look for `AddBot` in the log). `LoadLibrary` error 193 means you pointed at the 32-bit DLL.

Linux would use `omnibot_et.x86_64.so` once you have Linux `qagame`.

## Lua (optional)

Scripts run on the **server only**. If `lua_modules` is empty, Lua stays idle.

```
set lua_modules "yourscript.lua"
```

Put `.lua` files in `jaymod/`, `jaymod/luascripts/`, or `jaymod/lua/`. Console: `lua_status`, `lua_restart`.

The API follows the [published ET: Legacy Lua docs](https://etlegacy-lua-docs.readthedocs.io/en/latest/). The implementation is original (Apache 2.0). It is **not** a copy of Legacy’s GPLv3 `g_lua.c`. Admin subset only: chat commands, cvars, kicks, files, HTTP (`et.httpRequest` on Windows). No botlib / renderer / sound registration.

## What this fork changed (3.0.0)

- Native Lua 5.1 host (`lua_modules`, `lua_allowedModules`)
- 64-bit `qagame` / `cgame` / `ui` for ET: Legacy; same pk3 still serves 32-bit ET clients
- `vmMain` / `dllEntry` / syscalls use `intptr_t`
- Omni-bot Win64: load `omnibot_et_x64.dll`, MSVC-layout engine vtable from MinGW `qagame`, GameEntity returns match MSVC x64 (hidden pointer + address in `RAX`)
- Serverinfo trimmed so `protocol` is not truncated off the getinfo reply
- 32-bit client modules stamped **Jaymod 3.0.0** (leftover 2.3.0 `cgame` aborted on 32-bit ETL)

**2.3.0 (RNGesus)** is still the gameplay/timing baseline: weapon cooling, recoil, turn spread, steal time, flame range, antiwarp, spectator/score fixes, `g_panzerMapLimit`, admin DB autosave. Full list: [docs/changelog.md](docs/changelog.md).

## Known limits

- **Enhanced Mod** (`jaymod_enh.dll` / `.so`) will not attach. Recreate those commands in Lua or in `src/game/cmd/`.
- This zip has **no Linux client or server modules**.
- Lua is a server-admin subset of the published Legacy API.
- Stock ET 2.60b dedicated cannot load the 64-bit `qagame`.

## Build (Windows x64)

CMake 3.16+ and a 64-bit toolchain (this tree was built with MinGW-w64 / WinLibs UCRT).

```
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --target qagame --target cgame --target ui
```

Outputs: `build/qagame_mp_x64.dll`, `cgame_mp_x64.dll`, `ui_mp_x64.dll` (plus `*_x86_64` copies).

Pack the release zip:

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools/pack-release.ps1
```

Linux (same CMake, not in this archive): `qagame.mp.x86_64`, `cgame.mp.x86_64`, `ui.mp.x86_64`.

More: [docs/build.md](docs/build.md).

## Docs

| Page | Contents |
| --- | --- |
| [docs/server.md](docs/server.md) | Install, homepath, clients, Omni-bot, Enhanced Mod |
| [docs/lua.md](docs/lua.md) | Cvars, callbacks, `et.*` API |
| [docs/changelog.md](docs/changelog.md) | 2.2.0 → 2.3.0 → 3.0.0 |
| [docs/cvars.md](docs/cvars.md) | New cvars plus the classic 2.2.0 list |
| [docs/build.md](docs/build.md) | CMake, outputs, what not to use |

## Credits

- **Jaybird** — Jaymod 2.2.0 (2005–2011)
- **RNGesus** — 2.3.0 maintenance (2026)
- **Punkilla(MightyFinger77) This fork** — Lua host, 64-bit ET: Legacy modules, Win64 Omni-bot 0.93 (2026)

Original credits stay in the game.

## License

Apache License 2.0 and the original id Software terms. Keep both licenses and the credits with any redistribution.