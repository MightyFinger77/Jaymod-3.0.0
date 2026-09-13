# Server install

This is the 3.0.0 **64-bit ET: Legacy** server pack. Official Jaymod 2.2.0 was 32-bit `etded` only.

## What you need

- **ET: Legacy 2.85+** 64-bit `etlded.exe`
- `jaymod-3.0.0-etlded-server.zip`
- Your existing `jaymod` data: mapscripts, `shrubbot.cfg`, `user.db`, map cfgs

Minimum files from the zip:

| File | Required | |
| --- | --- | --- |
| `jaymod-3.0.0.pk3` | yes | Clients download this. Server opens `jaymod-3.0.0.dat` on init. |
| `qagame_mp_x64.dll` | yes | 64-bit dedicated game module |

Do **not** put `qagame` inside the pk3.

## Install

1. Copy `jaymod-3.0.0.pk3` and `qagame_mp_x64.dll` into the `jaymod` folder next to `etlded.exe` (or your `fs_homepath` jaymod folder).
2. Remove or stop using older `jaymod-2.2.0.pk3` / `jaymod-2.3.0.pk3` on that server so clients get **3.0.0**.
3. Keep mapscripts, shrubbot, and `user.db`.
4. Start:

```
etlded.exe +set fs_game jaymod +exec server.cfg
```

5. Leave `g_requireClientVersion 0` unless every player has this pk3.

`qagame_mp_x64.dll` must be the one from **this** zip. A leftover 32-bit `qagame_mp_x86.dll` is only useful if you still run 32-bit `etded`.

### Homepath vs basepath

ETL often searches `Documents\ETLegacy\jaymod` **before** `.\jaymod`. If the log shows:

```
Sys_LoadDll(...\Documents\ETLegacy\jaymod\qagame_mp_x64.dll)... failed
Sys_LoadDll(.\jaymod\qagame_mp_x64.dll)... succeeded
```

the working copy is the one under the server directory. Put the new DLL and pk3 in the folder that actually loaded.

## Clients (32-bit still works)

The pk3 contains **both** client bitnesses. The engine loads the pair that matches the player’s process.

| Player | Modules used |
| --- | --- |
| Vanilla ET 2.60b / 32-bit ETL | `cgame_mp_x86.dll`, `ui_mp_x86.dll` |
| 64-bit ETL | `cgame_mp_x64.dll`, `ui_mp_x64.dll` |
| 32-bit Linux | `cgame.mp.i386.so`, `ui.mp.i386.so` |

A 32-bit client never loads the x64 DLLs. They join the same 64-bit `etlded`.

This Windows pack does **not** include Linux 64-bit `cgame.mp.x86_64.so` / `ui.mp.x86_64.so`.

## Lua (optional)

See [Lua](lua.md).

```
set lua_modules "yourscript.lua"
```

Put scripts in `jaymod/` or `jaymod/luascripts/`. If you skip this, the server still runs; Lua stays idle.

## Omni-bot (optional)

ETL’s 64-bit archive ships **two** Windows libraries:

| File | Arch | Use with this qagame |
| --- | --- | --- |
| `omnibot_et.dll` | 32-bit | **No** — `LoadLibrary` error 193 |
| `omnibot_et_x64.dll` | 64-bit | **Yes** |

Copy the whole `legacy/omni-bot` folder (`omnibot_et_x64.dll`, `et/`, `global_scripts/`) and point at it:

```
set omnibot_enable "1"
set omnibot_path "C:/Servers/YourServer/omni-bot"
```

Use a real folder path. Do not point at an old 32-bit Pink/SteamCMD `omnibot` tree.

On a good load the console shows `OMNIBOT: load '...omnibot_et_x64.dll': success` then `initialization: success`.

Linux 64-bit: `omnibot_et.x86_64.so`.

## Enhanced Mod

`jaymod_enh.dll` / `.so` will **not** attach to this `qagame`. Recreate those commands in [Lua](lua.md) or in `src/game/cmd/`.

## Checklist

- [ ] 64-bit `etlded.exe`
- [ ] `jaymod-3.0.0.pk3` in the `jaymod` folder that the server actually searches
- [ ] `qagame_mp_x64.dll` from this 3.0.0 zip
- [ ] `+set fs_game jaymod`
- [ ] `g_requireClientVersion 0` unless you control every client pk3
- [ ] Omni-bot: `omnibot_et_x64.dll` + `omnibot_path` (if you want bots)
- [ ] Lua: `lua_modules` only if you want scripts
