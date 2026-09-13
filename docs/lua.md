# Lua

Jaymod 3.0.0 embeds **Lua 5.1** in `qagame`. Scripts run on the **server only**. Clients do not execute Lua.

The API follows the [published ET: Legacy Lua docs](https://etlegacy-lua-docs.readthedocs.io/en/latest/) (`et_*` callbacks, `et.*` functions). The implementation is original (Apache 2.0). It is **not** a copy of Legacy’s GPLv3 `g_lua.c`.

This is a **server-admin** subset: chat commands, cvars, kicks, files, HTTP. It does not expose botlib, renderer, or sound registration.

## Enable

Put `.lua` files in the `jaymod` folder, `jaymod/luascripts/`, or `jaymod/lua/`. List them in cfg:

```
set lua_modules "yourscript.lua"
set lua_allowedModules ""
```

`lua_modules` is space-separated. Up to **16** modules. Each file is at most **1 MB**.

Search order per name: `name`, `luascripts/name`, `lua/name`.

### Cvars

| Cvar | Default | Meaning |
| --- | --- | --- |
| `lua_modules` | `""` | Scripts to load. Empty = Lua is idle (VM is still compiled in). |
| `lua_allowedModules` | `""` | If set, only scripts whose SHA1 hex is in this string are loaded. Empty = allow all. |

### Console

| Command | Who | What |
| --- | --- | --- |
| `lua_status` | server / rcon | List loaded modules |
| `lua_restart` | server / rcon | Unload and reload `lua_modules` |

Changing `lua_modules` or `lua_allowedModules` at runtime also triggers a reload.

## What you can do

Typical uses:

- Chat commands (`!rules`, custom `!` commands) via `et_ClientCommand`
- Server console commands via `et_ConsoleCommand`
- Print / center-print / `cpm` / `chat` with `et.trap_SendServerCommand`
- Read and set cvars
- Kick / drop clients
- Read userinfo (name, guid, ip)
- Read/write files in the mod homepath
- POST to Discord or other HTTP APIs (`et.httpRequest`, Windows)
- React to connect, spawn, damage, and obituaries
- Read/write a subset of `gentity` / client fields

Enhanced Mod commands that no longer attach can be recreated here.

## Callbacks

Define only the functions you need. Return **1** from command hooks to **eat** the command (Jaymod will not process it further). Return **0** to pass it through.

| Function | Arguments | Return |
| --- | --- | --- |
| `et_InitGame` | `levelTime, randomSeed, restart` | — |
| `et_ShutdownGame` | `restart` | — |
| `et_Quit` | — | called on Lua shutdown |
| `et_RunFrame` | `levelTime` | — |
| `et_ClientConnect` | `clientNum, firstTime, isBot` | string = reject reason; `nil` = allow |
| `et_ClientDisconnect` | `clientNum` | — |
| `et_ClientBegin` | `clientNum` | — |
| `et_ClientUserinfoChanged` | `clientNum` | — |
| `et_ClientSpawn` | `clientNum, revived, teamChange, restoreHealth` | — |
| `et_ClientCommand` | `clientNum, command` | `1` = handled |
| `et_ConsoleCommand` | (use `et.trap_Argv`) | `1` = handled |
| `et_Obituary` | `victim, killer, meansOfDeath` | — |
| `et_Damage` | `target, attacker, damage, dflags, mod` | — |

Chat like `say !hello` arrives as `et_ClientCommand` with `command == "say"`. Read the text with `et.ConcatArgs(1)` or `et.trap_Argv`.

## `et.*` functions

### Module / print

| Function | Notes |
| --- | --- |
| `et.RegisterModname(name)` | Shown in `lua_status` |
| `et.FindSelf()` | This VM’s slot id |
| `et.FindMod(name)` | Slot id or `-1` |
| `et.G_Print(text)` | Server console |
| `et.G_Printf(fmt, ...)` | `string.format` then print |
| `et.G_LogPrint(text)` | Game log |

### Args, cvars, configstrings

| Function | Notes |
| --- | --- |
| `et.trap_Argc()` | |
| `et.trap_Argv(i)` | |
| `et.ConcatArgs(startIndex)` | Joined args from `startIndex` |
| `et.trap_Cvar_Get(name)` | |
| `et.trap_Cvar_Set(name, value)` | |
| `et.trap_GetConfigstring(index)` | |
| `et.trap_SetConfigstring(index, value)` | |
| `et.trap_Milliseconds()` | |

### Commands and players

| Function | Notes |
| --- | --- |
| `et.trap_SendConsoleCommand(when, text)` | `when`: `et.EXEC_NOW`, `et.EXEC_INSERT`, `et.EXEC_APPEND` |
| `et.trap_SendServerCommand(clientNum, text)` | `-1` = all. Examples: `"cpm \"Hi\"\n"`, `"chat \"Hi\"\n"` |
| `et.trap_DropClient(clientNum, reason, length)` | |
| `et.ClientNumberFromString(s)` | Slot or `-1` |
| `et.G_Say(clientNum, mode, text)` | `et.SAY_ALL`, `et.SAY_TEAM`, `et.SAY_BUDDY` |
| `et.trap_GetUserinfo(clientNum)` | |
| `et.trap_SetUserinfo(clientNum, info)` | |
| `et.ClientUserinfoChanged(clientNum)` | Call after `SetUserinfo` |

### Info strings

| Function | Notes |
| --- | --- |
| `et.Info_ValueForKey(info, key)` | |
| `et.Info_SetValueForKey(info, key, value)` | returns new string |
| `et.Info_RemoveKey(info, key)` | returns new string |
| `et.Q_CleanStr(s)` | Strip color codes |

### Files

| Function | Notes |
| --- | --- |
| `et.trap_FS_FOpenFile(path, mode)` | returns `handle, length`. Modes: `et.FS_READ`, `et.FS_WRITE`, `et.FS_APPEND`, `et.FS_APPEND_SYNC` |
| `et.trap_FS_Read(handle, len)` | |
| `et.trap_FS_Write(data, len, handle)` | |
| `et.trap_FS_FCloseFile(handle)` | |
| `et.trap_FS_Rename(from, to)` | |
| `et.trap_FS_GetFileList(path, ext)` | |

Paths are relative to the ET/Jaymod file system (mod folder / homepath), not arbitrary disk paths.

### Entities

```lua
local name = et.gentity_get(clientNum, "pers.netname")
local origin = et.gentity_get(clientNum, "ps.origin")  -- { x, y, z }
et.gentity_set(clientNum, "health", 100)
local ammo = et.gentity_get(clientNum, "ps.ammo", WP_MP40)  -- array field + index
```

Readable / writable fields:

| Field | Type | Notes |
| --- | --- | --- |
| `classname` | string | |
| `inuse` | int | read-only |
| `spawnflags`, `flags`, `health`, `damage`, `splashDamage`, `splashRadius`, `count`, `timestamp`, `nextthink` | int | |
| `target`, `targetname`, `message`, `model` | string | |
| `sess.sessionTeam` | int | `et.TEAM_*` |
| `sess.spectatorState` | int | |
| `pers.netname` | string | read-only |
| `pers.connected`, `pers.enterTime` | int | read-only |
| `ps.origin`, `ps.velocity`, `ps.viewangles` | vec3 | table of 3 numbers |
| `ps.stats`, `ps.persistant`, `ps.powerups`, `ps.ammo`, `ps.ammoclip` | int array | pass index as 3rd arg |
| `ps.weapon` | int | |
| `noclip` | int | |

### HTTP (Windows)

```lua
local status, body = et.httpRequest(url, method, body, headers)
```

| Arg | Default | |
| --- | --- | --- |
| `url` | required | `http://` or `https://` |
| `method` | `"GET"` | `GET`, `POST`, … |
| `body` | `""` | |
| `headers` | `{}` | map, e.g. `{ ["Content-Type"] = "application/json" }` |

Returns HTTP status (or `0` on failure) and response body. Timeout is 4 seconds. Not implemented on non-Windows builds (returns `0, ""`).

## Constants

Registered on the `et` table: `EXEC_NOW`, `EXEC_INSERT`, `EXEC_APPEND`, `FS_READ`, `FS_WRITE`, `FS_APPEND`, `FS_APPEND_SYNC`, `SAY_ALL`, `SAY_TEAM`, `SAY_BUDDY`, `TEAM_FREE`, `TEAM_AXIS`, `TEAM_ALLIES`, `TEAM_SPECTATOR`, `CS_PLAYERS`, `CS_SERVERINFO`, `CS_SYSTEMINFO`, `MOD_UNKNOWN`, `MOD_MACHINEGUN`, `MOD_SUICIDE`, `MAX_CLIENTS`.

Weapon and MOD enums beyond those few are not registered; use numeric values from Jaymod/`bg_public.h` if you need them.

## Minimal script

```lua
function et_InitGame(levelTime, randomSeed, restart)
    et.RegisterModname("hello 1.0")
    et.G_Print("hello.lua loaded\n")
end

function et_ClientCommand(clientNum, command)
    if string.lower(et.trap_Argv(0) or "") ~= "say" then
        return 0
    end
    local text = et.ConcatArgs(1) or ""
    if string.lower(string.sub(text, 1, 6)) ~= "!hello" then
        return 0
    end
    et.trap_SendServerCommand(-1, "cpm \"Hello from Lua.\"\n")
    return 1
end
```

## Compatibility notes

- Scripts written for ET: Legacy often work if they only use the functions listed above.
- Legacy-only calls (`et.G_Damage`, `et.trap_Trace`, sound, SHA1 helpers, …) are **not** bound yet; the script will error if it calls them.
- Lua cannot replace Enhanced Mod’s native overlay. Recreate the commands you care about.
- Do not put secrets in `lua_modules` itself; keep tokens in a cvar or a file the script reads.
