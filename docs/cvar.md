# Jaymod 3.1.0 cvar list

Server cvars for **Jaymod 3.1.0**. Values in the **jaymod.cfg** column are from the bundled sample (`pkg/jaymod.cfg.m4`). Put `exec jaymod.cfg` in `server.cfg`. Add the 3.0.0 / 3.1.0 blocks yourself if your copy of `jaymod.cfg` is older.

Engine default is listed when it differs from the sample. Empty string is `""`. Bitflags are sums of the listed bits.

Original XML pages for classic cvars are under `doc/cvar/`. Lua details: [lua.md](lua.md). Map vote: [server.md](server.md#map-voting).

## Added in 3.0.0

Lua host and 64-bit Omni-bot. Not in the classic sample cfg.

| Cvar | Default | What it does |
| --- | --- | --- |
| `lua_modules` | `""` | Space-separated Lua scripts to load from the mod folder, `luascripts/`, or `lua/`. Empty = Lua stays idle. |
| `lua_allowedModules` | `""` | If set, only scripts whose SHA1 hex appears in this string are loaded. Empty = allow all. |
| `lua_reportWebhook` | `""` | Optional HTTP URL for scripts. Do not put secrets in `jaymod.cfg`; keep tokens in a file the script reads. |
| `omnibot_enable` | `1` | Load Omni-bot. Use ETL’s 64-bit `omnibot_et_x64.dll`, not 32-bit `omnibot_et.dll`. |
| `omnibot_path` | `""` | Folder that contains `omnibot_et_x64.dll`, `et/`, and `global_scripts/`. |
| `omnibot_flags` | `0` | Omni-bot option flags. |
| `omnibot_playing` | `0` | Read-only count of bots in serverinfo. |

```
set lua_modules ""
set lua_allowedModules ""
set omnibot_enable "1"
set omnibot_path "C:/path/to/omni-bot"
```

Console: `lua_status`, `lua_restart`.

## Added in 3.1.0

EnhMod-in-qagame, Nitmod-style map vote, widescreen HUD.

### Map voting (`g_gametype 6`)

Keep gametype 6 on every map in the cycle. `g_xpSave` still works.

| Cvar | Default | What it does |
| --- | --- | --- |
| `g_gametype` | sample `2`, engine `4` | `2` objective, `3` stopwatch, `4` campaign, `5` LMS, **`6` map vote**. |
| `g_maxMapsVotedFor` | `0` | How many maps on the intermission list. `0` = every map that is not excluded (max 96), including the map just played. |
| `g_minMapAge` | `3` | Hide maps played this many maps ago. Ignored when `g_maxMapsVotedFor` is `0`. |
| `g_excludedMaps` | `:oasis:goldrush:radar:railgun:fueldump:` | Colon-separated bsp names to hide. Add `:battery:` to hide Battery too. |
| `g_mapVoteFlags` | `20` | `1` least-played wins ties, `2` wait until `g_intermissionReadyPercent` have voted, `4` three ranked votes (3/2/1), `8` do not shuffle, `16` `!nextmap` / callvote nextmap stay on the vote screen. Default `20` is `4+16`. Ranked votes are always on in 3.1.0. |

```
set g_gametype 6
set g_excludedMaps ":oasis:goldrush:radar:railgun:fueldump:"
set g_maxMapsVotedFor "0"
set g_minMapAge "3"
set g_mapVoteFlags "20"
```

### EnhMod (published 1.0.9d names)

| Cvar | Default | What it does |
| --- | --- | --- |
| `g_em_votemap` | `0` | `1` limits callvote to `map`, `nextmap`, `maprestart`, and `campaign`. |
| `g_adrenenalinecls` | `2` | Published spelling. Bits: `1` soldier, `2` medic, `4` engineer, `8` fieldops, `16` covert. Default `2` = medics. |
| `g_drawAttackerHP` | `0` | Show last attacker HP: `1` console, `2` on-screen CPM. |
| `g_countryflags` | `0` | `1` writes country id into userinfo when `GeoLite2-City.mmdb`, `GeoLite2-Country.mmdb`, or legacy `GeoIP.dat` is next to `qagame`. |
| `g_flagsbehaviour` | `""` | `random` = random bot flag. Otherwise treated as an IP to look up for bots. |
| `g_rifleWar` | `0` | Rifle-only spawn (`!riflewar`). |
| `g_pistolWar` | `0` | Pistol-only spawn (`!pistolwar`). |
| `g_pumpgunWar` | `0` | M97-only spawn (`!pumpgunwar`). |

Spawn / ammo / antirush live in `ModEnhConfig.xml` and `enhmod_*.db`, not cvars. `modifieddoublejump` in XML turns on `g_misc` bit `1`.

### Client (3.1.0)

| Cvar | Default | What it does |
| --- | --- | --- |
| `jay_fixedAspect` | `1` | `1` or unset = widescreen HUD/UI without stretch. `0` = old stretched 640×480. ETL’s archived `cg_fixedAspect 0` is ignored. |

## Added in 2.3.0 (RNGesus)

Still used in 3.1.0. Most are not in the classic sample cfg.

| Cvar | Default | What it does |
| --- | --- | --- |
| `g_requireClientVersion` | `0` | `1` rejects clients whose Jaymod version does not match. Keep `0` if older pk3s are still in the wild. |
| `g_dbAutoSave` | `300` | Seconds between automatic Admin DB saves. `0` disables. |
| `g_aimSpreadTurnScale` | `1.0` | Scales turn spread. `1.0` is the frame-rate-independent amount. |
| `g_hitmodeAntilagInterp` | `0` | Shifts antilag reconciliation back by this many ms. |
| `g_playerCount` | `0` | Read-only Axis + Allied player count for map scripts. |
| `g_panzerMapLimit` | `0` | Caps consecutive maps one player can hold a panzer. |
| `g_debugBullets` | `0` | Dev-only bullet path overlay (cheats / `devmap`). |
| `cg_cycleBinoculars` | `0` | Client. `0` skips binoculars when cycling weapons. |

`cg_noVoiceChats` is a bit field as of 2.3.0: `0` all, `1` none, `3` team/fireteam only.

---

## Classic sample `jaymod.cfg`

### Security

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `g_password` | `""` | Password for connecting clients. Engine default is `none`. |
| `g_shoutcastpassword` | `""` | Password for shoutcaster login. |
| `rconpassword` | `""` | Remote console password. |
| `refereePassword` | `""` | Client referee promotion password. |
| `sv_privatePassword` | `""` | Password for reserved private slots. |

### Logging

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `g_log` | `""` | Game log file. Empty = off. |
| `g_logOptions` | `0` | Bits: `1` log chat, `2` extended weapons, `8` bans. |
| `g_logSync` | `0` | Flush the log after each write. |
| `g_adminLog` | `""` | Admin command log file. |

### Branding

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `sv_hostname` | `ETHost` | Name in the server browser. |
| `g_watermark` | `""` | Client watermark shader. Engine default `jaymod`. |
| `g_watermarkFadeAfter` | `60` | Seconds before the watermark starts fading. |
| `g_watermarkFadeTime` | `60` | Fade duration in seconds. |
| `g_protestMessage` | `Visit www.myserver.com to file a protest.` | Footer on punishment disconnects. |
| `g_kickMessage` | `You have been kicked for $TIME.` | Kick text. `$TIME` is replaced. |
| `g_kickTime` | `2m` | Temp-ban length after a kick. Engine default `2m`. |

`sets .NAME` / `sets .URL` in the sample are optional serverinfo keys (commented out).

### MOTD

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `server_motd0` | `""` | MOTD line 0. Engine default is an ET MOTD string. |
| `server_motd1` | `""` | MOTD line 1. |
| `server_motd2` | `""` | MOTD line 2. |
| `server_motd3` | `""` | MOTD line 3. |
| `server_motd4` | `""` | MOTD line 4. |
| `server_motd5` | `""` | MOTD line 5. |

### Registration

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `sv_master1` | `etmaster.idsoftware.com` | Master server. ETL servers often use `etmaster.etlegacy.com`. |
| `sv_master2` | `""` | Extra master. |
| `sv_master3` | `""` | Extra master. |
| `sv_master4` | `""` | Extra master. |
| `sv_master5` | `""` | Extra master. |

### Networking

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `sv_allowDownload` | `1` | Direct pk3 download from the server. |
| `sv_dl_maxRate` | `42000` | Max rate for those downloads. |
| `sv_floodProtect` | `1` | Client command flood protection. |
| `sv_fps` | `20` | Server tick rate. |
| `sv_fullmsg` | `Server is full.` | Message when `sv_maxclients` is reached. |
| `sv_lanForceRate` | `1` | Force LAN client rates. |
| `sv_maxPing` | `0` | Max ping to join. `0` = no limit. |
| `sv_maxRate` | `13000` | Max bandwidth per client. Engine default in qagame is `25000`. |
| `sv_maxclients` | `20` | Max connected clients. |
| `sv_minPing` | `0` | Min ping to join. `0` = no limit. |
| `sv_packetdelay` | `0` | Simulated extra latency (debug). |
| `sv_packetloss` | `0` | Simulated packet loss (debug). |
| `sv_padPackets` | `0` | Packet padding. |
| `sv_privateClients` | `4` | Reserved slots (need `sv_privatePassword`). |
| `sv_pure` | `1` | Client purity check. |
| `sv_reconnectlimit` | `3` | Min seconds between reconnects. |
| `sv_showAverageBPS` | `0` | Print bandwidth stats. |
| `sv_showloss` | `0` | Log lost usercmds. |
| `sv_timeout` | `240` | Drop silent clients after this many seconds. |
| `sv_wwwBaseURL` | `""` | HTTP download prefix. |
| `sv_wwwDlDisconnected` | `0` | Disconnect the client while HTTP downloading. |
| `sv_wwwDownload` | `0` | Enable HTTP downloads. |
| `sv_wwwFallbackURL` | `""` | Fallback URL if HTTP download fails. |
| `sv_zombietime` | `2` | Seconds a dropped slot stays a zombie. |

### Voting

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `vote_allow_balancedteams` | `1` | Allow balanced-teams votes. |
| `vote_allow_comp` | `1` | Allow competition-settings votes. |
| `vote_allow_friendlyfire` | `1` | Allow friendly-fire votes. |
| `vote_allow_gametype` | `1` | Allow gametype votes. |
| `vote_allow_generic` | `1` | Allow generic votes. |
| `vote_allow_kick` | `1` | Allow kick votes. |
| `vote_allow_map` | `1` | Allow map votes. |
| `vote_allow_matchreset` | `1` | Allow match-reset votes. |
| `vote_allow_matchrestart` | `1` | Allow match-restart votes. |
| `vote_allow_mutespecs` | `1` | Allow mute-spectators votes. |
| `vote_allow_muting` | `1` | Allow mute votes. |
| `vote_allow_nextmap` | `1` | Allow nextmap votes. |
| `vote_allow_pub` | `1` | Allow public-settings votes. |
| `vote_allow_referee` | `0` | Allow referee votes. |
| `vote_allow_shuffleteamsxp` | `1` | Allow shuffle-by-XP votes. |
| `vote_allow_swapteams` | `1` | Allow swap-teams votes. |
| `vote_allow_timelimit` | `0` | Allow timelimit votes. |
| `vote_allow_warmupdamage` | `1` | Allow warmup-damage votes. |
| `vote_limit` | `5` | Max votes a player may call. |
| `vote_percent` | `50` | Percent yes needed to pass. |

Not in the sample cfg, still registered:

| Cvar | Default | What it does |
| --- | --- | --- |
| `vote_allow_startmatch` | `1` | Allow start-match votes. |
| `vote_voteBased` | `0` | `1` counts only players who voted. |
| `vote_minPercent` | `0` | With `vote_voteBased`, min percent of players who must vote. |
| `g_em_votemap` | `0` | 3.1.0: restrict which votes exist. See above. |

### Banners

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `g_bannerLocation` | `4` | Where banners print. Engine default `0`. |
| `g_bannerTime` | `60` | Seconds each banner is shown. Engine default `5`. |
| `g_banners` | `2` | How many `g_bannerN` strings to cycle. |
| `g_banner1` | `^3THIS SERVER IS RUNNING …` | First banner. |
| `g_banner2` | `^3Check forums at …` | Second banner. |

Add `g_banner3`, `g_banner4`, … if `g_banners` is higher.

### Matchplay

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `g_gametype` | `2` | See 3.1.0 map-vote section. Engine default `4` (campaign). |
| `g_campaignFile` | `""` | Alternate campaign file. |
| `g_headshot` | `0` | Bits: `1` headshots only, `2` instagib headshots. |
| `g_knifeonly` | `0` | Knife-only mode. |
| `g_panzerWar` | `0` | Panzer-war mode. |
| `g_sniperWar` | `0` | Sniper-war mode. |
| `match_latejoin` | `1` | Allow joining a match already in progress. |
| `match_minplayers` | `0` | Players required before the match starts. |
| `match_mutespecs` | `0` | Mute spectators (also blocks spec vsay to players). |
| `match_readypercent` | `100` | Percent ready needed to start. |
| `match_timeoutcount` | `3` | Non-referee timeouts allowed. |
| `match_timeoutlength` | `180` | Timeout length in seconds. |
| `match_warmupDamage` | `1` | Damage during warmup. |

### Teams

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `g_userAlliedRespawnTime` | `0` | Override Allied wave time (ms). `0` = map default. |
| `g_userAxisRespawnTime` | `0` | Override Axis wave time (ms). |
| `g_teamForceBalance` | `1` | Block joining the larger team. Engine default `0`. |
| `g_ammoRechargeTime` | `60000` | Ammo cabinet respawn (ms). |
| `g_healthRechargeTime` | `10000` | Health cabinet respawn (ms). |
| `team_maxArtillery` | `6` | Artillery / airstrikes per minute. |
| `team_maxLandMines` | `20` | Landmines per team. Engine default `10`. |
| `team_maxFlamers` | `-1` | Flamethrowers per team. `-1` = no limit. |
| `team_maxGrenLaunchers` | `-1` | Rifle grenades per team. |
| `team_maxM97s` | `-1` | M97s per team. |
| `team_maxMG42s` | `-1` | MG42s per team. |
| `team_maxMortars` | `-1` | Mortars per team. |
| `team_maxPanzers` | `-1` | Panzers per team. |
| `team_maxplayers` | `0` | Max players per team. `0` = no extra cap. |
| `team_maxMedics` | `-1` | Medics per team. |
| `team_maxEngineers` | `-1` | Engineers per team. |
| `team_maxFieldOps` | `-1` | Field ops per team. |
| `team_maxCovertOps` | `-1` | Covert ops per team. |
| `team_nocontrols` | `1` | Disable team-issued pause / ready controls. |

### Players / skills / classes

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `g_defaultSkills` | `0 0 0 0 0 0 0` | Starting skill levels for new players. |
| `g_levels_battlesense` | `20 50 90 140 200` | XP rungs. Empty in engine = stock rungs. |
| `g_levels_covertops` | `20 50 90 140 200` | XP rungs. |
| `g_levels_engineer` | `20 50 90 140 200` | XP rungs. |
| `g_levels_fieldops` | `20 50 90 140 200` | XP rungs. |
| `g_levels_lightweapons` | `20 50 90 140 200` | XP rungs. |
| `g_levels_medic` | `20 50 90 140 200` | XP rungs. |
| `g_levels_soldier` | `20 50 90 140 200` | XP rungs. |
| `g_covertops` | `0` | Covert bits: `1` keep disguise, `2` medkit, `4` ammo, `8` mines, `16` live uniform steal, `32` draw name. |
| `g_engineers` | `0` | Engineer bits: `1` friendly mines, `2` friendly dynamite, `4` share XP. |
| `g_medics` | `0` | Medic bits: `4` less regen, `8` no regen, `16` share adrenaline, `32` delayed regen. |
| `g_soldiers` | `0` | Soldier bits: `1` panzer gravity. |
| `g_sk5_battle` | `1` | Level-5 battlesense bits. `1` faster stamina. |
| `g_sk5_cvops` | `7` | Level-5 covert: `1` less charge, `2` extra nades, `4` poison gas. |
| `g_sk5_eng` | `127` | Level-5 engineer (charge, nades, mine spot/defuse, construct, bouncing betty, poison mines). |
| `g_sk5_fdops` | `3` | Level-5 field ops: `1` less charge, `2` extra nades. |
| `g_sk5_lightweap` | `1` | Level-5 light weapons: `1` less recoil. |
| `g_sk5_medic` | `243` | Level-5 medic (charge, nades, class carry-over regen). |
| `g_sk5_soldier` | `7` | Level-5 soldier: `1` less charge, `2` extra nades, `4` poison gas. |
| `g_covertopsChargeTime` | `30000` | Covert charge (ms). |
| `g_engineerChargeTime` | `30000` | Engineer charge (ms). |
| `g_LTChargeTime` | `40000` | Field ops charge (ms). |
| `g_soldierChargeTime` | `20000` | Soldier charge (ms). |
| `g_medicChargeTime` | `45000` | Medic charge (ms). |
| `g_medicSelfHealDelay` | `0` | Medic self-heal delay (ms). |

### Bulletmode / hitmode

| Cvar | jaymod.cfg | What it does |
| --- | --- | --- |
| `g_bulletmode` | `0` | Active bullet model. |
| `g_bulletmodeDebug` | `0` | Bulletmode debug flags. |
| `g_bulletmodeReference` | `1` | Reference bullet model for comparison. |
| `g_bulletmodeTrail` | `0` | Max bullet trails to draw. |
| `g_hitmode` | `0` | Active hit model. |
| `g_hitmodeAntilag` | `800` | Max antilag (ms). |
| `g_hitmodeAntilagLerp` | `1` | Lerp antilag positions. |
| `g_hitmodeAntilagInterp` | (not in sample; default `0`) | Extra ms subtracted from antilag time. 2.3.0. |
| `g_hitmodeDebug` | `0` | Hitmode debug flags. |
| `g_hitmodeFat` | `0` | Extra torso box size (inches). |
| `g_hitmodeGhosting` | `0` | Hit ghost lifetime (ms). |
| `g_hitmodeReference` | `1` | Reference hit model. |
| `g_hitmodeZone` | `0` | Debug hit zone. Engine default `1`. |

### Miscellaneous (sample)

| Cvar | jaymod.cfg | Engine default | What it does |
| --- | --- | --- | --- |
| `g_admin` | `1` | `""` | Turn the admin system on. |
| `g_alliedmaxlives` | `0` | `0` | Allied lives. `0` = unlimited. |
| `g_altStopwatchMode` | `0` | `0` | Alternate stopwatch. |
| `g_antiwarp` | `1` | `1` | `1` enable antiwarp, `32` log warping. |
| `g_autoFireteams` | `0` | `1` | Auto-place players in fireteams. |
| `g_axismaxlives` | `0` | `0` | Axis lives. |
| `g_censor` | `0` | `0` | Word filter. |
| `g_censorPenalty` | `0` | `0` | Bits: `1` kill, `2` kick if in name, `4` no gib, `8` temp mute. |
| `g_classChange` | `0` | `0` | Steal class from a friendly corpse. |
| `g_complaintlimit` | `6` | `6` | Complaints per map before kick. |
| `g_damagexp` | `0` | `0` | Award XP from damage dealt. |
| `g_debugAlloc` | `0` | `0` | Memory debug. |
| `g_debugConstruct` | `0` | `0` | Smaller construct charge (cheat). |
| `g_debugDamage` | `0` | `0` | Damage debug (cheat). |
| `g_debugMove` | `0` | `0` | Movement debug. |
| `g_debugSkills` | `0` | `0` | Skills debug. |
| `g_disableComplaints` | `0` | `0` | Disable TK complaints for some weapons. |
| `g_dragCorpse` | `1` | `0` | Drag corpses. |
| `g_dropAmmo` | `2` | `0` | Ammo packs dropped on field-ops death. |
| `g_dropHealth` | `2` | `0` | Health packs dropped on medic death. |
| `g_dynamiteTime` | `30` | `30` | Dynamite fuse (seconds). |
| `g_enforcemaxlives` | `1` | `1` | Temp-ban reconnects that dodge max lives. |
| `g_fastres` | `0` | `0` | Faster revives. |
| `g_fear` | `0` | `0` | Award a kill if the victim `/kill`s to escape. |
| `g_filterBan` | `1` | `1` | Apply IP bans. |
| `g_filtercams` | `0` | `0` | Hide players from camera views. |
| `g_fixedPhysics` | `1` | `1` | Fixed physics (`g_fixedphysics`). |
| `g_fixedPhysicsFPS` | `125` | `125` | Emulated FPS for fixed physics. |
| `g_forcerespawn` | `0` | `0` | Force limbo after this many seconds. `0` = off. |
| `g_friendlyFire` | `1` | `1` | Friendly fire. |
| `g_glow` | `0` | `0` | Colored player glow. |
| `g_goomba` | `4` | `0` | Damage from landing on a player. |
| `g_gravity` | `800` | `800` | Gravity. |
| `g_heavyWeaponRestriction` | `100` | `100` | Heavy-weapon percent cap per team. |
| `g_inactivity` | `0` | `0` | Kick idle players after this many seconds. `0` = off. |
| `g_intermissionReadyPercent` | `75` | `100` | Percent ready (or voted, on mapvote) to end intermission. |
| `g_intermissionTime` | `30` | `60` | Intermission length (seconds). Map vote uses this as the vote timer. |
| `g_ipcomplaintlimit` | `3` | `3` | Unique IPs that can complain about one player. |
| `g_killSpreeLevels` | `5 10 15 20 25 30` | `""` | Kill-spree thresholds. |
| `g_killingSpree` | `1` | `0` | `1` sprees, `2` also records. |
| `g_knockback` | `1000` | `1000` | Knockback. |
| `g_landminetimeout` | `1` | `1` | Remove a player’s mines on disconnect. |
| `g_lms_followTeamOnly` | `1` | `1` | LMS: spectate own team only. |
| `g_lms_lockTeams` | `0` | `0` | LMS: lock teams during a match. |
| `g_lms_matchlimit` | `2` | `2` | LMS matches before nextmap. |
| `g_lms_roundlimit` | `3` | `3` | LMS rounds per match. |
| `g_lms_teamForceBalance` | `1` | `1` | LMS team balance. |
| `g_loseSpreeLevels` | `10 20 30` | `""` | Losing-spree thresholds. |
| `g_mapConfigs` | `mapconfigs` | `""` | Folder of per-map `.cfg` files. |
| `g_mapScriptDirectory` | `mapscripts` | `""` | Alternate mapscript folder. |
| `g_maxGameClients` | `0` | `0` | Max players in the game (spectators extra). `0` = no cap. |
| `g_maxlives` | `0` | `0` | Lives for everyone. `0` = unlimited. |
| `g_maxlivesRespawnPenalty` | `0` | `0` | Extra wait after lives run out. |
| `g_misc` | `66` | `0` | Bits: `1` double jump, `2` binoc-war, `4` admins only, `8` throw packs up, `32` BS4 full revive, `64` realistic aim spread. Sample `66` = `2+64`. |
| `g_moverScale` | `1.0` | `1.0` | Door / mover speed scale. |
| `g_movespeed` | `76` | `76` | Player move speed (cheat cvar). |
| `g_muteTime` | `0` | `0` | Default mute length. `0` = until unmute. |
| `g_noTeamSwitching` | `0` | `0` | Block team changes during a match. |
| `g_packDistance` | `4` | `1` | Health/ammo throw distance multiplier. |
| `g_playDead` | `1` | `0` | Allow play-dead. |
| `g_poisonSyringes` | `1` | `0` | Poison syringes. |
| `g_proneDelay` | `0` | `0` | Extra prone delay. |
| `g_privateMessages` | `1` | `0` | `/m` private messages. |
| `g_reflectFriendlyFire` | `100` | `100` | Percent of FF reflected to the attacker. |
| `g_saveCampaignStats` | `1` | `1` | Keep stats across campaign maps. |
| `g_scriptDebug` | `0` | `0` | Mapscript parse debug (cheat). |
| `g_scriptDebugLevel` | `0` | `0` | Mapscript runtime debug (cheat). |
| `g_scriptName` | `""` | `""` | Force a mapscript name (cheat). |
| `g_shortcuts` | `0` | `0` | Chat shortcuts (`[a]`, `[d]`, …). |
| `g_shove` | `100` | `0` | Shove distance. `0` = off. |
| `g_shoveNoZ` | `1` | `0` | No vertical shove. |
| `g_skills` | `0` | `0` | Extra skill-system flags. |
| `g_slashKill` | `0` | `0` | `/kill`: `1` half charge, `2` empty charge, `3` keep charge, `4` disable `/kill`. |
| `g_smoothClients` | `1` | `1` | Smooth missed client frames. |
| `g_snap` | `7` | `7` | Snap player/item origins. |
| `g_spawnInvul` | `3` | `3` | Spawn invuln (seconds). |
| `g_spectator` | `0` | `0` | Bits: `1` click to follow, `2` click-miss, `4` persist, `8` free spec. |
| `g_spectatorInactivity` | `0` | `0` | Kick idle spectators. `0` = off. |
| `g_speed` | `320` | `320` | Baseline player speed. |
| `g_teamDamageMinHits` | `6` | `6` | Hits before team-damage restriction. |
| `g_teamDamageRestriction` | `0` | `0` | Team-damage percent that triggers restriction. `0` = off. |
| `g_truePing` | `1` | `1` | True ping calculation. |
| `g_voiceChatsAllowed` | `4` | `4` | Voice chats per 30 seconds. |
| `g_vulnerableWeapons` | `0` | `0` | Shootable projectiles: `1` panzer, `2` grenade, `4` canister, `8` satchel. |
| `g_warmup` | `30` | `60` | Warmup seconds. |
| `g_weapons` | `5606` | `0` | See bits below. Sample enables underwater tools, helmet packs, drop binocs, fair rifles, throwing knives, M97, molotovs. |
| `g_wolfrof` | `0` | `0` | Original Wolfenstein rates of fire. |
| `g_xpCap` | `0` | `0` | At XP cap: `0` still gain, `1` stop, `2` reset. |
| `g_xpMax` | `0` | `0` | XP cap amount. `0` = none. |
| `g_xpSave` | `1` | `0` | `1` save XP, `2` also reset between campaigns. |
| `g_xpSaveTimeout` | `1h` | `1h` | How long saved XP lasts. |

`g_weapons` bits:

| Bit | |
| --- | --- |
| `1` | Field ops without battlesense spawn without binoculars |
| `2` | Syringes work underwater |
| `4` | Pliers work underwater |
| `8` | Full charge refund on “too many airstrikes” |
| `16` | Half charge refund on “too many airstrikes” |
| `32` | Ammo packs restore a lost helmet |
| `64` | Drop binoculars on death |
| `128` | Allied rifles reload mid-clip like Axis |
| `256` | Throwing knives |
| `512` | Poison throwing knives |
| `1024` | Winchester M97 |
| `2048` | Disable adrenaline |
| `4096` | Molotov cocktails |

## Registered, not in the sample cfg

| Cvar | Default | What it does |
| --- | --- | --- |
| `g_bluelimbotime` | `30000` | Allied wave time (ms). Maps often override this. |
| `g_redlimbotime` | `30000` | Axis wave time (ms). |
| `timelimit` | `0` | Map time limit (minutes). |
| `fraglimit` | `0` | Unused in Wolf objective. |
| `g_doWarmup` | `0` | Warmup enable (legacy). |
| `nextmap` | `""` | Console command run when the map ends with no vote winner. |
| `nextcampaign` | `""` | Next campaign. |
| `g_banIPs` | `""` | Legacy IP ban list. |
| `g_userTimeLimit` | `0` | Override timelimit. |
| `g_minGameClients` | `8` | Shown in serverinfo. |
| `g_maxMapsVotedFor` | `0` | 3.1.0 map vote. |
| `g_minMapAge` | `3` | 3.1.0 map vote. |
| `g_excludedMaps` | `:oasis:goldrush:radar:railgun:fueldump:` | 3.1.0 map vote. |
| `g_mapVoteFlags` | `20` | 3.1.0 map vote. |
| `g_requireClientVersion` | `0` | 2.3.0. |
| `g_dbAutoSave` | `300` | 2.3.0. |
| `g_playerCount` | `0` | ROM. |
| `g_panzerMapLimit` | `0` | 2.3.0. |
| `g_aimSpreadTurnScale` | `1.0` | 2.3.0. |
| `g_ammoUnlimited` | `0` | Unlimited ammo. |
| `g_ammoFireDelayNudge` | `0` | Fire-delay nudge. |
| `g_ammoNextDelayNudge` | `0` | Next-shot delay nudge. |
| `g_warnDecay` | `1` | Warn points decay. |
| `g_warnMuteLevel` | `50` | Warn score that mutes. |
| `g_warnBanLevel` | `100` | Warn score that bans. |
| `g_shutdownExit` | `0` | Exit the process on shutdown. |
| `dedicated` | `0` | `1` listen, `2` dedicated. Set by the engine. |
| `sv_uptime` | `""` | ROM uptime string. |
| `sv_cpu` | `""` | ROM CPU string in serverinfo. |
| `g_needpass` | `0` | ROM: password required. |
| `g_balancedteams` | `0` | ROM team-balance flag. |
| `g_antilag` | `1` | ROM; hitmode antilag is the real control. |
| `bot_enable` | `0` | Legacy bot cvar. Use `omnibot_enable`. |

## Client notes

These are not in `jaymod.cfg` (server). Players set them locally.

| Cvar | Default | What it does |
| --- | --- | --- |
| `jay_fixedAspect` | `1` | 3.1.0 widescreen HUD. |
| `cg_cycleBinoculars` | `0` | 2.3.0 weapon cycle. |
| `cg_noVoiceChats` | `0` | `0` all, `1` none, `3` team/fireteam only. |
| `cg_hitsounds` | (client) | Hit sounds. |
| `cg_althud` | (client) | Alternate HUD. |
| `cg_fixedAspect` | ETL archive often `0` | Ignored. Use `jay_fixedAspect`. |
