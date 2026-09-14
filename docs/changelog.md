# Changelog

Lineage: official **Jaymod 2.2.0** (Jaybird, 2011) → **2.3.0** (RNGesus, 2026) → **3.0.0** (was 2.3.1: Lua + 64-bit ET: Legacy) → **3.1.0** (was 2.3.2: EnhMod built into qagame).

## 3.1.0

Formerly numbered **2.3.2**.

Same platform as 3.0.0, plus **EnhMod 1.0.9d behaviour inside `qagame`**. There is no `jaymod_enh.dll`. Drop the published files next to `qagame`:

| File | Role |
| --- | --- |
| `ModEnhConfig.xml` | `<common>` + `<entity method="add\|if\|remove">` spawn rules + optional `<weaponammo>` magazine/reserve tiers |
| `enhmod_commands.db` | Custom `!command` / `exec` / `levels` |
| `enhmod_level.db` | EnhMod levels and flag letters |
| `enhmod_admin.db` | GUID → level / flags (not shipped) |
| `enhmod_antirush.db` | Map spheres: `name/key/team/coord/radius/timeout/method` |
| `forcecvarfile.cfg` | `forcecvar` and `forcecvarex` |
| `commands_flags.txt` | Builtin flag letters from the 1.0.9d package |

Builtins (`!riflewar`, `!pistolwar`, `!pumpgunwar`, `!freeze`, `!unfreeze`, `!disarm`, `!warn`, `!country`, `!impact`, `!crazydisguise`, `!midget`, `!antirush`, `!antirush_add`, `!antirush_del`) accept **either** Jaymod shrubbot letter **`M`** **or** the EnhMod flag from `commands_flags.txt`. Custom commands use `enhmod_admin.db` GUID → level, else Jaymod `authLevel`.

Cvars keep the published names, including the `g_adrenenalinecls` spelling. `g_em_votemap 1` allows only `map`, `nextmap`, `maprestart`, and `campaign`. Antirush `method` `r` and `j` are stored; both teleport to the team spawn before the timeout. Country uses free `GeoLite2-City.mmdb` (country + city) or `GeoLite2-Country.mmdb`. Each operator downloads their own from MaxMind; they are not shipped (we cannot redistribute them). If both files are present, City is used. Legacy `GeoIP.dat` still works. The connect IP/country line is not public: only levels whose flags include **`I`** see it (plus the server console). `advancedplayerinfo` still has to be on. `modifieddoublejump` turns on Jaymod `g_misc` `MISC_DOUBLEJUMP` — no extra pmove physics.

`jay_fixedAspect` (default `1`) is the widescreen HUD/UI switch. ETL often archives `cg_fixedAspect 0`, so that cvar is **not** used. Unset or `1` keeps 4:3 element sizes: HUD uses the extra width (left stuff stays left, right stuff stays right), menus / limbo / tab / load and exit screens stay a centered 640×480 panel, and in-game text (chat, kill feed, SPECTATOR, scoreboard names) uses TTF so it does not stretch. Centered HUD strings (first blood, center print, objective print, Connection Interrupted, crosshair names) use real TTF width instead of 8px cells. Tab/intermission win art sits on the centered 4:3 board. `jay_fixedAspect 0` is the old stretched 640×480 look.

Spectators can open the **V** vsay menu and send global vsay. Team / fireteam vsay stays blocked. Server `match_mutespecs` still decides whether players hear spec voice.

Optional `<weaponammo>` in `ModEnhConfig.xml` sets magazine (`maxclip`) and reserve (`maxammo`) caps per weapon. List `<tier>`s in order; the highest matching tier wins. Each `<need>` on a tier is AND. `xp` is that skill’s XP (not total XP). `xp="max"` is the last enabled rung of `g_levels_*`. This is not spawn starting ammo (`<ammo>` / `<ammoclip>` on `<entity>`). If a weapon has tiers, the vanilla skill-1 extra clip at spawn is skipped. Existing `ModEnhConfig.xml` is not overwritten — add the block yourself. See [server install](server.md#weapon-ammo-tiers).

`jaymod_enh.dll` still will not attach. Keep `g_requireClientVersion 0` if 2.3.0 / 3.0.0 clients are still joining.

## 3.0.0

Formerly numbered **2.3.1**. Server reports version **3.0.0**. Client modules in the pk3 are 2.3.0 assets plus 64-bit `cgame`/`ui` and `jaymod-3.0.0.dat`. Keep `g_requireClientVersion 0` if players still run an older Jaymod pk3.

### Lua

- Vendored Lua 5.1 in `qagame`
- `lua_modules`, `lua_allowedModules`
- ET: Legacy–style callbacks (`et_InitGame`, `et_ClientCommand`, …) and `et.*` functions
- `et.httpRequest` on Windows (WinHTTP) for HTTP admin tools
- Console: `lua_status`, `lua_restart`

See [Lua](lua.md).

### 64-bit ET: Legacy

- CMake builds `qagame_mp_x64.dll`, `cgame_mp_x64.dll`, `ui_mp_x64.dll` (and `*_x86_64` copies)
- `vmMain` / `dllEntry` / engine syscalls use `intptr_t`
- `jaymod-3.0.0.pk3` ships **both** 32-bit and 64-bit client modules
- 32-bit `cgame_mp_x86.dll` / `ui_mp_x86.dll` must be stamped to the same `jver` as the server (3.1.0 now). A leftover 2.3.0 / 3.0.0 module aborts in `CG_Init` on ET: Legacy 32-bit, unloads cgame, then `STATUS_BAD_STACK`
- `qagame` stays **outside** the pk3
- Omni-bot loader tries `omnibot_et_x64.dll` (Windows) and `omnibot_et.x86_64.so` (Linux) before the 32-bit names

See [server install](server.md) and [build](build.md).

### Known limits in 3.0.0

- Enhanced Mod (`jaymod_enh`) will not attach to this `qagame`
- Linux 64-bit *clients* need `.x86_64.so` modules (the Windows zip does not include those)
- Lua is a server-admin subset of the published Legacy API (no botlib / sound / shader registration)

## 2.3.0 (RNGesus)

Maintenance release. Bug fixes, not new weapons or modes. A lot of 2.2.0 counted frames where it should have counted milliseconds, so behavior silently depended on client FPS and `sv_fps`.

Source: [RngesusSolutions/jaymod2.2.0](https://github.com/RngesusSolutions/jaymod2.2.0). Precompiled zip: [jaymod-2.3.0.zip](https://github.com/RngesusSolutions/jaymod2.2.0/releases/download/2.3.0/jaymod-2.3.0.zip).

### Timing / frame-rate

- Weapon cooling keeps the fractional remainder between frames (high FPS no longer overheats early)
- STEN `maxHeat` 1200 → 1350 so a full burst is still 16 rounds after the cooling fix
- Recoil on a fixed 15 ms schedule
- Turn spread keeps fractional changes (`g_aimSpreadTurnScale`)
- Uniform stealing is 2.5 real seconds
- Flame chunks use real frame duration (`sv_fps` no longer changes range)
- `pmove_fixed` leaning crash fixed
- Fixed-movement clients no longer pile commands in antiwarp, inflate ping, or get false Connection Interrupted

### Shooting, movement, hits

- Crouched bullets and `+activate` traces start at real eye height
- Emplaced MG42 fire lines up with the crosshair
- Crouch-spam scripts are rate-limited
- Prone clearance keeps the traced leg offset
- Overkill XP only for health actually removed
- Dead players cannot shove; dead or feigning players cannot be shoved
- `cg_crosshairX` / `cg_crosshairY` are real screen pixels at any resolution
- `g_hitmodeAntilagInterp` offsets antilag reconciliation
- `g_debugBullets` draws real paths (devmap / cheats)

### Weapons and equipment

- Thrown knives stop bouncing after a while
- Satchel detonator checks the correct clip value
- Weapon cycling skips binoculars by default (`cg_cycleBinoculars 1` restores 2.2.0)
- `g_panzerMapLimit` rotates panzer access across maps
- Tank / emplaced-gun mounting is rate-limited
- “Insufficient fire support” spam is throttled
- Spotted landmines stay visible to engineer defuse traces
- Mines spotted in one batch share one announcement and score update

### Spectators, teams, scores

- Spectators keep their own score
- Players waiting to spawn can only follow their own team
- Limbo followers move when their target switches teams
- Scope FOV/reticle is cleared after following a scoped player
- A full `g_maxGameClients` only blocks a spectator *joining*; class/team change no longer dumps you to spec
- End-of-map XP awards use that map’s XP, not career totals
- Per-map revives and headshots reset properly
- Stale stats are not restored when a new stats period starts
- Leftover projectiles stop damaging after the owner disconnects

### Chat, voice, UI

- Ignore list covers voice chat
- `cg_noVoiceChats` is a bit field (`3` = team/fireteam only)
- Lagometer draws all 48 samples at every resolution
- Crosshair and no-shoot indicators share the pixel-accurate offset

### Server

- `g_dbAutoSave` — timed Admin DB save
- `g_requireClientVersion` — optional matching client pk3
- `g_playerCount` — live player count for map scripts
- Weapon-to-item lookups cached
- Command-map entity messages use fixed buffers
- Axis and Allied landmines counted in one pass
- Event-count overflow fails at build time

### `g_panzerMapLimit`

With `g_panzerMapLimit 3`, a player can take a panzer on three maps, then is locked out for the next three completed maps.

- A map counts only if the weapon was actually granted
- Voluntarily skipping before the limit resets the run
- Stored in `user.db` by GUID
- Lockout counts server-completed maps (spec / offline still count)
- `map_restart` does not advance
- Still applies with `team_maxPanzers -1` or `g_panzerWar 1`
- Players without a GUID are not tracked

### 2.3.0 cvars

| Cvar | Default | What it does |
| --- | --- | --- |
| `g_requireClientVersion` | `0` | `1` kicks mismatched Jaymod client versions |
| `g_dbAutoSave` | `300` | Seconds between Admin DB saves; `0` off |
| `g_aimSpreadTurnScale` | `1.0` | Turn-spread scale (sent to clients) |
| `g_hitmodeAntilagInterp` | `0` | Extra ms of antilag rewind |
| `g_playerCount` | `0` | Read-only player count |
| `g_panzerMapLimit` | `0` | Consecutive-map panzer cap |
| `g_debugBullets` | `0` | Draw bullet paths (cheats) |
| `cg_cycleBinoculars` | `0` | `1` = old cycle-through-binocs |

## 2.2.0 (Jaybird)

Last official Jaymod. Admin, shrubbot, extra weapons (M97, molotov, poison, …), Jaymod HUD/UI, Omni-bot 0.8 interface. 32-bit ET 2.60b only.

Site: [jaymod.clanfu.org](https://jaymod.clanfu.org/).
