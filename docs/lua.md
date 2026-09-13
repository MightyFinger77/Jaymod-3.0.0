# Lua

Jaymod 3.1.0 embeds **Lua 5.1** in `qagame`. Scripts run on the **server only**. Clients do not execute Lua.

The API follows the [published ET: Legacy Lua docs](https://etlegacy-lua-docs.readthedocs.io/en/latest/) (`et_*` callbacks, `et.*` functions). The implementation is original (Apache 2.0). It is **not** a copy of Legacy’s GPLv3 `g_lua.c`.

This is a **server** API: chat commands, cvars, kicks, files, HTTP, damage/XP, sound, traces, and entity spawn. It does not expose botlib or the renderer.

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
- React to connect, spawn, damage, obituaries, client think, and weapon fire
- Damage, XP, mute, weapons, sounds, traces, and entity spawn
- Read/write `gentity` / client fields (kills, ping, skill points, origin, …)

EnhMod 1.0.9d is built into `qagame` in 3.1.0. Lua is still available for extra admin scripts.

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
| `et_ClientThink` | `clientNum` | every command from that client |
| `et_WeaponFire` | `clientNum, weapon` | after dead/playdead checks |
| `et_IPCReceive` | `fromVm, message` | sent by `et.IPCSend` |

Chat like `say !hello` arrives as `et_ClientCommand` with `command == "say"`. Read the text with `et.ConcatArgs(1)` or `et.trap_Argv`.

## `et.*` functions

### Module / print

| Function | Notes |
| --- | --- |
| `et.RegisterModname(name)` | Shown in `lua_status` |
| `et.FindSelf()` | This VM’s slot id |
| `et.FindMod(slot)` | `modname, signature` (or `nil, nil`) |
| `et.FindModByName(name)` | Slot id or `-1` |
| `et.IPCSend(slot, message)` | Calls `et_IPCReceive` on that VM; returns `1`/`0` |
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
| `et.trap_Cvar_GetInteger(name)` | |
| `et.trap_GetConfigstring(index)` | |
| `et.trap_SetConfigstring(index, value)` | |
| `et.trap_GetServerinfo()` | |
| `et.trap_Milliseconds()` | |
| `et.trap_RealTime()` | table: `tm_sec`, `tm_min`, `tm_hour`, … |
| `et.G_GetLevelTime()` | `level.time` |
| `et.G_GetStartTime()` | `level.startTime` |

### Commands and players

| Function | Notes |
| --- | --- |
| `et.trap_SendConsoleCommand(when, text)` | `when`: `et.EXEC_NOW`, `et.EXEC_INSERT`, `et.EXEC_APPEND` |
| `et.trap_SendServerCommand(clientNum, text)` | `-1` = all. Examples: `"cpm \"Hi\"\n"`, `"chat \"Hi\"\n"` |
| `et.trap_DropClient(clientNum, reason, length)` | |
| `et.ClientNumberFromString(s)` | Slot, or `nil` if not exactly one match |
| `et.PlayerName(clientNum)` | `pers.netname` or `nil` |
| `et.IsConnected(clientNum)` | `1` / `0` |
| `et.G_Say(clientNum, mode, text)` | `et.SAY_ALL`, `et.SAY_TEAM`, `et.SAY_BUDDY` |
| `et.MutePlayer(clientNum, reason)` | reason optional |
| `et.UnmutePlayer(clientNum)` | also `et.UnMutePlayer` |
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
| `et.Q_stricmp(a, b)` | `0` if equal |
| `et.G_SHA1(s)` | hex digest; aliases `et.sha1`, `et.SHA1` |
| `et.isBitSet(bit, value)` | `1` if bit is set |

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

### Damage, XP, weapons

| Function | Notes |
| --- | --- |
| `et.G_Damage(target, inflictor, attacker, damage, dflags, mod)` | Legacy 6-arg form. `dir`/`point` tables optional as args 7–8 |
| `et.G_Damage(target, inflictor, attacker, dir, point, damage, dflags, mod)` | 8-arg form when arg 4 is a table |
| `et.G_XP_Set(clientNum, xp, skill, update)` | Sets `sess.skillpoints[skill]`, then rank. `update` ignored |
| `et.G_ResetXP(clientNum)` | Same as Jaymod XP reset |
| `et.G_AddSkillPoints(clientNum, skill, points)` | |
| `et.G_LoseSkillPoints(clientNum, skill, points)` | |
| `et.AddWeaponToPlayer(clientNum, weapon, ammo, clip, setcurrent)` | returns `1`/`0` |
| `et.RemoveWeaponFromPlayer(clientNum, weapon)` | |
| `et.GetCurrentWeapon(clientNum)` | `ps.weapon` |
| `et.COM_BitCheck(clientNum, weapon)` | test `ps.weapons` |
| `et.COM_BitSet(clientNum, weapon)` | |
| `et.COM_BitClear(clientNum, weapon)` | |

### Sound and indexes

| Function | Notes |
| --- | --- |
| `et.G_SoundIndex(path)` | |
| `et.G_ModelIndex(path)` | |
| `et.G_ShaderIndex(path)` | |
| `et.G_Sound(entnum, soundIndex)` | |
| `et.G_globalSound(path)` | all clients |
| `et.G_ClientSound(clientNum, pathOrIndex)` | string path or numeric index |

### Trace / world

| Function | Notes |
| --- | --- |
| `et.trap_Trace(start, mins, maxs, end, passEntityNum, mask)` | returns table: `fraction`, `endpos`, `entityNum`, `allsolid`, `startsolid`, `surfaceFlags`, `contents`, `plane` |
| `et.trap_PointContents(point, passEntityNum)` | |
| `et.trap_InPVS(a, b)` | boolean |
| `et.trap_EntitiesInBox(mins, maxs)` | array of entity numbers |

`start` / `mins` / `maxs` / `end` / `point` are `{x,y,z}` tables. Defaults: `passEntityNum = et.ENTITYNUM_NONE`, `mask = et.MASK_SHOT`.

### Entities

```lua
local name = et.gentity_get(clientNum, "pers.netname")
local origin = et.gentity_get(clientNum, "ps.origin")  -- { x, y, z }
et.gentity_set(clientNum, "health", 100)
local ammo = et.gentity_get(clientNum, "ps.ammo", WP_MP40)  -- array field + index
```

| Function | Notes |
| --- | --- |
| `et.G_Spawn()` | new entity number, or `-1` |
| `et.G_FreeEntity(entnum)` | not for client slots |
| `et.G_EntitiesFree()` | unused entity count |
| `et.G_TempEntity(origin, event)` | entity number |
| `et.G_AddEvent(entnum, event, eventParm)` | |
| `et.G_SetOrigin(entnum, origin)` | |
| `et.G_SetAngle(entnum, angles)` | |
| `et.G_SetEntState(entnum, state)` | `et.STATE_*` |
| `et.trap_LinkEntity(entnum)` | |
| `et.trap_UnlinkEntity(entnum)` | |
| `et.G_Find(from, field, match)` | `field`: `classname`, `targetname`, `target`, `message`. `from` nil to start |
| `et.G_UseTargets(entnum, activator)` | |
| `et.AddRemap(oldShader, newShader, timeOffset)` | applies `CS_SHADERSTATE` |

Readable / writable fields:

| Field | Type | Notes |
| --- | --- | --- |
| `classname` | string | read-only |
| `inuse` | int | read-only |
| `spawnflags`, `flags`, `health`, `damage`, `splashDamage`, `splashRadius`, `count`, `count2`, `timestamp`, `nextthink`, `clipmask`, `takedamage`, `methodOfDeath`, `splashMethodOfDeath`, `waterlevel` | int | |
| `wait`, `random`, `delay` | float | |
| `target`, `targetname`, `message`, `model` | string | read-only |
| `parent`, `enemy`, `activator` | entity | entity number or `nil` |
| `s.origin`, `s.angles`, `s.origin2`, `s.pos.trBase`, `s.pos.trDelta`, `s.apos.trBase` | vec3 | |
| `s.number`, `s.eType`, `s.eFlags`, `s.weapon`, `s.teamNum`, `s.event`, `s.eventParm` | int | `s.number` read-only |
| `r.currentOrigin`, `r.currentAngles`, `r.mins`, `r.maxs` | vec3 | |
| `r.contents`, `r.ownerNum`, `r.svFlags` | int | |
| `r.linked` | int | read-only |
| `sess.sessionTeam` | int | `et.TEAM_*` |
| `sess.spectatorState`, `sess.spectatorClient`, `sess.playerType`, `sess.playerWeapon`, `sess.playerWeapon2` | int | |
| `sess.kills`, `sess.deaths`, `sess.suicides`, `sess.team_kills`, `sess.team_damage`, `sess.damage_given`, `sess.damage_received` | int | |
| `sess.referee`, `sess.rank`, `sess.revives`, `sess.headshots`, `sess.shoutcaster` | int | |
| `sess.skill` | int array | `et.SK_*` index |
| `sess.skillpoints` | float array | `et.SK_*` index |
| `pers.netname`, `pers.playerName` | string | read-only |
| `pers.connected`, `pers.enterTime` | int | read-only |
| `ps.origin`, `ps.velocity`, `ps.viewangles` | vec3 | |
| `ps.stats`, `ps.persistant`, `ps.powerups`, `ps.ammo`, `ps.ammoclip` | int array | pass index as 3rd arg |
| `ps.weapon`, `ps.pm_type`, `ps.pm_flags`, `ps.viewheight` | int | |
| `ps.ping`, `ps.clientNum` | int | read-only |
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

Registered on the `et` table (use as `et.WP_MP40`, `et.SK_LIGHT_WEAPONS`, …):

- Exec / files / say / teams: `EXEC_*`, `FS_*`, `SAY_*`, `TEAM_*`
- Configstrings: `CS_SERVERINFO`, `CS_SYSTEMINFO`, `CS_WARMUP`, `CS_INTERMISSION`, `CS_MULTI_INFO`, `CS_MULTI_MAPWINNER`, `CS_SHADERSTATE`, `CS_PLAYERS`, …
- Limits: `MAX_CLIENTS`, `MAX_GENTITIES`, `ENTITYNUM_NONE`, `ENTITYNUM_WORLD`
- Connect / spec / class: `CON_*`, `SPECTATOR_*`, `PC_*`
- Skills: `SK_BATTLE_SENSE` … `SK_NUM_SKILLS`
- Stats / pmove / ent state: `STAT_*`, `PM_*`, `STATE_*`
- Damage / contents: `DAMAGE_*`, `CONTENTS_*`, `MASK_*`, `SVF_*`
- Weapons: `WP_NONE` … `WP_NUM_WEAPONS` (Jaymod extras included: `WP_M97`, `WP_POISON_GAS`, `WP_MOLOTOV`, …)
- Means of death: `MOD_*` including `MOD_GOOMBA`, `MOD_M97`, `MOD_MOLOTOV`, …

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

- Scripts written for ET: Legacy often work if they use the functions listed above.
- Still missing Legacy-only pieces: mapscript `G_CreateEntity` / spawn-var helpers, `G_SetGlobalFog`, WolfAdmin internals, `et_Print` (would recurse).
- `et.G_ClientSound` accepts a **filename** (Jaymod) or a sound **index** (Legacy).
- `et.FindMod(slot)` matches Legacy (name + signature). Use `et.FindModByName` if you have a name.
- EnhMod 1.0.9d is built into `qagame`. Lua is for extra scripts, not a replacement for `jaymod_enh.dll`.
- Do not put secrets in `lua_modules` or `jaymod.cfg`. Keep tokens in a file the script reads.
