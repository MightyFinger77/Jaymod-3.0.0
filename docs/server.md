# Server install

This is the 3.1.0 **64-bit ET: Legacy** server pack. Official Jaymod 2.2.0 was 32-bit `etded` only.

## What you need

- **ET: Legacy 2.85+** 64-bit `etlded.exe`
- `jaymod-3.1.0-64bit-lua.zip`
- Your existing `jaymod` data: mapscripts, `shrubbot.cfg`, `user.db`, map cfgs

Minimum files from the zip:

| File | Required | |
| --- | --- | --- |
| `jaymod-3.1.0.pk3` | yes | Clients download this. Server opens `jaymod-3.1.0.dat` on init. |
| `qagame_mp_x64.dll` | yes | 64-bit dedicated game module |

Do **not** put `qagame` inside the pk3.

## Install

1. Copy `jaymod-3.1.0.pk3` and `qagame_mp_x64.dll` into the `jaymod` folder next to `etlded.exe` (or your `fs_homepath` jaymod folder).
2. Remove or stop using older `jaymod-2.2.0.pk3` / `jaymod-2.3.0.pk3` / `jaymod-2.3.1.pk3` / `jaymod-2.3.2.pk3` / `jaymod-3.0.0.pk3` on that server so clients get **3.1.0**.
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

Widescreen HUD is in the client modules. `jay_fixedAspect` defaults to `1`. ETL’s archived `cg_fixedAspect 0` does not turn it off. Set `jay_fixedAspect 0` only if you want the old stretched look. Centered in-game messages (first blood, center print, objectives, Connection Interrupted) use real TTF width so they sit on the screen center, not the old 8px-cell estimate.

Spectators can press **V** for the vsay menu and send global vsay. Team / fireteam vsay stays blocked. `match_mutespecs` still decides whether players hear spec voice.

## Lua (optional)

See [Lua](lua.md).

```
set lua_modules "example.lua"
```

Put scripts in `jaymod/`, `jaymod/luascripts/`, or `jaymod/lua/`. If you skip this, the server still runs; Lua stays idle.

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

EnhMod 1.0.9d is **built into this `qagame`**. Do not load `jaymod_enh.dll` / `.so`.

Put the published files next to `qagame` (`ModEnhConfig.xml`, `enhmod_*.db`, `forcecvarfile.cfg`). Builtin flag letters are in `commands_flags.txt`. Jaymod shrubbot letter **`M`** also grants those builtins. Custom commands use `enhmod_admin.db` / `enhmod_level.db` levels. Connect IP/country/version is shown only to levels whose flags include **`I`** (and the server console). `advancedplayerinfo` must be true. Add `I` to existing `enhmod_level.db` admin flags if that file was already on the server.

qagame will not overwrite an existing `ModEnhConfig.xml`. Add new blocks yourself.

Country / GeoIP is **not in the zip**. Each operator downloads their own free MaxMind GeoLite2 files (binary `.mmdb`) and puts them next to `qagame`. `GeoLite2-City.mmdb` gives country + city. `GeoLite2-Country.mmdb` is country only. If both are present, City is used. We cannot redistribute those files. See [changelog](changelog.md).

### Weapon ammo tiers

Optional `<weaponammo>` sets magazine (`maxclip`) and reserve (`maxammo`) **caps**. It is not spawn starting ammo. Spawn amounts stay on `<entity>` as `<ammo>` / `<ammoclip>`.

List `<tier>`s in order. The highest matching tier wins. Every `<need>` on a tier is AND. `xp` is that skill’s XP, not total XP. `xp="max"` is the last enabled rung of that skill (`g_levels_*`). Up to 6 tiers per gun, 3 needs per tier.

Skill names: `light_weapons`, `medic`, `engineer`, `soldier`, `fieldops`, `covertops`, `battle_sense`.

```xml
<weaponammo>
	<weapon name="WP_MP40">
		<tier maxclip="30" maxammo="90"/>
		<tier maxclip="30" maxammo="120">
			<need skill="light_weapons" xp="200"/>
		</tier>
		<tier maxclip="45" maxammo="360">
			<need skill="light_weapons" xp="max"/>
			<need skill="battle_sense" xp="max"/>
		</tier>
	</weapon>
</weaponammo>
```

That example is 30/90 at 0 XP, 30/120 at 200 Light Weapons, 45/360 at max Light Weapons **and** max Battle Sense.

The old one-line form still works. `skill` / `skill2` are OR:

```xml
<weapon name="WP_THOMPSON" maxclip="30" maxammo="90" maxammo_skilled="120" skill="light_weapons" skill2="medic" xp="20"/>
```

If a weapon has `<weaponammo>` tiers, the vanilla skill-1 extra clip at spawn is skipped so it does not bypass your gates. Clients need this 3.1.0 pk3 so reload prediction matches the server.

## Checklist

- [ ] 64-bit `etlded.exe`
- [ ] `jaymod-3.1.0.pk3` in the `jaymod` folder that the server actually searches
- [ ] `qagame_mp_x64.dll` from this 3.1.0 zip
- [ ] `+set fs_game jaymod`
- [ ] `g_requireClientVersion 0` unless you control every client pk3
- [ ] Omni-bot: `omnibot_et_x64.dll` + `omnibot_path` (if you want bots)
- [ ] Lua: `lua_modules` only if you want scripts
