# Jaymod cvar reference

Original Jaymod 2.2.0 server cvars, plus notes for later releases. Names link to the XML pages under `doc/cvar/`.

Cvars added or changed after 2.2.0 are listed first. The long table below is the classic set.

## Added or changed after 2.2.0

| Cvar | Default | Since | What it does |
| --- | --- | --- | --- |
| `lua_modules` | `""` | 3.0.0 | Space-separated Lua scripts to load from the mod folder, `luascripts/`, or `lua/`. |
| `lua_allowedModules` | `""` | 3.0.0 | If set, only scripts whose SHA1 appears in this list are loaded. Empty = allow all. |
| `g_requireClientVersion` | `0` | 2.3.0 | `1` rejects clients whose Jaymod version does not match the server. Keep `0` if 2.2.0 / 2.3.0 pk3s are still in the wild. |
| `g_dbAutoSave` | `300` | 2.3.0 | Seconds between automatic Admin DB saves. `0` disables. |
| `g_aimSpreadTurnScale` | `1.0` | 2.3.0 | Scales turn spread. `1.0` is the frame-rate-independent amount. |
| `g_hitmodeAntilagInterp` | `0` | 2.3.0 | Shifts antilag reconciliation back by this many ms. |
| `g_playerCount` | `0` | 2.3.0 | Read-only Axis + Allied player count for map scripts. |
| `g_panzerMapLimit` | `0` | 2.3.0 | Caps consecutive maps one player can hold a panzer. |
| `g_debugBullets` | `0` | 2.3.0 | Dev-only bullet path overlay (cheats / devmap). |
| `cg_cycleBinoculars` | `0` | 2.3.0 | `0` skips binoculars when cycling weapons. |

`cg_noVoiceChats` is a bit field as of 2.3.0: `0` all, `1` none, `3` team/fireteam only.

See [changelog](changelog.md) for `g_panzerMapLimit` rules.

## Classic 2.2.0 set

Original Jaymod cvar list (one line each). Names link to `doc/cvar/` XML pages.

### Core server, network, and access cvars

| Cvar | Default | What it does |
| --- | --- | --- |
| [`dedicated`](../doc/cvar/cvar.dedicated.xml) | `0` | Sets server mode. |
| [`omnibot_enable`](../doc/cvar/cvar.omnibot_enable.xml) | `1` | Turns Omni-bot support on or off. |
| [`rconpassword`](../doc/cvar/cvar.rconpassword.xml) | `""` | Sets password to enable remote console commands. |
| [`refereePassword`](../doc/cvar/cvar.refereePassword.xml) | `""` | Sets password for client referee promotion. |
| [`server_motd0`](../doc/cvar/cvar.server_motd0.xml) | `""` | Sets server message-of-the-day. |
| [`server_motd1`](../doc/cvar/cvar.server_motd1.xml) | `""` | Sets server message-of-the-day. |
| [`server_motd2`](../doc/cvar/cvar.server_motd2.xml) | `""` | Sets server message-of-the-day. |
| [`server_motd3`](../doc/cvar/cvar.server_motd3.xml) | `""` | Sets server message-of-the-day. |
| [`server_motd4`](../doc/cvar/cvar.server_motd4.xml) | `""` | Sets server message-of-the-day. |
| [`server_motd5`](../doc/cvar/cvar.server_motd5.xml) | `""` | Sets server message-of-the-day. |
| [`sv_allowDownload`](../doc/cvar/cvar.sv_allowDownload.xml) | `1` | Turns direct client download on or off. |
| [`sv_dl_maxRate`](../doc/cvar/cvar.sv_dl_maxRate.xml) | `42000` | Sets max rate for direct client downloads. |
| [`sv_floodProtect`](../doc/cvar/cvar.sv_floodProtect.xml) | `1` | Turns client commands flood protection on or off. |
| [`sv_fps`](../doc/cvar/cvar.sv_fps.xml) | `20` | Sets server frequency. |
| [`sv_fullmsg`](../doc/cvar/cvar.sv_fullmsg.xml) | `"Server is full."` | Sets server-full message. |
| [`sv_hostname`](../doc/cvar/cvar.sv_hostname.xml) | `"ETHost"` | Sets name of server shown in browse lists. |
| [`sv_lanForceRate`](../doc/cvar/cvar.sv_lanForceRate.xml) | `1` | Turns automatic network settings for LAN clients on or off. |
| [`sv_master1`](../doc/cvar/cvar.sv_master1.xml) | `"etmaster.idsoftware.com"` | Sets master server for browse-list registration. |
| [`sv_master2`](../doc/cvar/cvar.sv_master2.xml) | `""` | Sets supplemental master server for browse-list registration. |
| [`sv_master3`](../doc/cvar/cvar.sv_master3.xml) | `""` | Sets supplemental master server for browse-list registration. |
| [`sv_master4`](../doc/cvar/cvar.sv_master4.xml) | `""` | Sets supplemental master server for browse-list registration. |
| [`sv_master5`](../doc/cvar/cvar.sv_master5.xml) | `""` | Sets supplemental master server for browse-list registration. |
| [`sv_maxclients`](../doc/cvar/cvar.sv_maxclients.xml) | `20` | Sets maximum number of connected clients. |
| [`sv_maxPing`](../doc/cvar/cvar.sv_maxPing.xml) | `0` | Sets maximum allowable client ping. |
| [`sv_maxRate`](../doc/cvar/cvar.sv_maxRate.xml) | `13000` | Sets maximum network bandwidth per client. |
| [`sv_minPing`](../doc/cvar/cvar.sv_minPing.xml) | `0` | Sets minimum required client ping. |
| [`sv_packetdelay`](../doc/cvar/cvar.sv_packetdelay.xml) | `0` | Sets simulated server latency. |
| [`sv_packetloss`](../doc/cvar/cvar.sv_packetloss.xml) | `0` | Sets simulated server packet loss. |
| [`sv_padPackets`](../doc/cvar/cvar.sv_padPackets.xml) | `0` | Sets packet padding amount. |
| [`sv_privateClients`](../doc/cvar/cvar.sv_privateClients.xml) | `4` | Sets number of reserved client slots. |
| [`sv_privatePassword`](../doc/cvar/cvar.sv_privatePassword.xml) | `""` | Sets password for reserved private player slots. |
| [`sv_pure`](../doc/cvar/cvar.sv_pure.xml) | `1` | Turns client purity check on or off. |
| [`sv_reconnectlimit`](../doc/cvar/cvar.sv_reconnectlimit.xml) | `3` | Sets minimum period required between client reconnections. |
| [`sv_showAverageBPS`](../doc/cvar/cvar.sv_showAverageBPS.xml) | `0` | Prints average server bandwidth statistics for debugging. |
| [`sv_showloss`](../doc/cvar/cvar.sv_showloss.xml) | `0` | Turns lost usercmd logging on or off. |
| [`sv_timeout`](../doc/cvar/cvar.sv_timeout.xml) | `240` | Sets client network connection timeout. |
| [`sv_wwwBaseURL`](../doc/cvar/cvar.sv_wwwBaseURL.xml) | `""` | Sets URL download prefix for WWW downloads of server files. |
| [`sv_wwwDlDisconnected`](../doc/cvar/cvar.sv_wwwDlDisconnected.xml) | `0` | Controls whether clients disconnect from the server while downloading. |
| [`sv_wwwDownload`](../doc/cvar/cvar.sv_wwwDownload.xml) | `0` | Turns HTTP download on or off. |
| [`sv_wwwFallbackURL`](../doc/cvar/cvar.sv_wwwFallbackURL.xml) | `""` | Sets URL for failed WWW downloads. |
| [`sv_zombietime`](../doc/cvar/cvar.sv_zombietime.xml) | `2` | Sets zombie period. |

### Gameplay and Jaymod cvars

| Cvar | Default | What it does |
| --- | --- | --- |
| [`g_admin`](../doc/cvar/cvar.g_admin.xml) | `0` | Turns admin system on or off. |
| [`g_adminLog`](../doc/cvar/cvar.g_adminLog.xml) | `""` | Sets filename used for admin command logging. |
| [`g_alliedmaxlives`](../doc/cvar/cvar.g_alliedmaxlives.xml) | `0` | Sets maximum number of lives for Allied players. |
| [`g_altStopwatchMode`](../doc/cvar/cvar.g_altStopwatchMode.xml) | `0` | Turns alternative stopwatch gametype on or off. |
| [`g_ammoRechargeTime`](../doc/cvar/cvar.g_ammoRechargeTime.xml) | `60000` | Sets time interval between ammo-pack cabinet respawns. |
| [`g_antiwarp`](../doc/cvar/cvar.g_antiwarp.xml) | `1` | Controls the flags for antiwarp functionality. |
| [`g_autoFireteams`](../doc/cvar/cvar.g_autoFireteams.xml) | `0` | Turns automatic fireteam placement on or off. |
| [`g_axismaxlives`](../doc/cvar/cvar.g_axismaxlives.xml) | `0` | Sets maximum number of lives for Axis players. |
| [`g_bannerLocation`](../doc/cvar/cvar.g_bannerLocation.xml) | `0` | Sets banner location. |
| [`g_banners`](../doc/cvar/cvar.g_banners.xml) | `0` | Sets number of banners to display. |
| [`g_bannerTime`](../doc/cvar/cvar.g_bannerTime.xml) | `5` | Sets the duration of display for each banner. |
| [`g_bluelimbotime`](../doc/cvar/cvar.g_bluelimbotime.xml) | `30000` | Sets the time between Allied team respawns. |
| [`g_bulletmode`](../doc/cvar/cvar.g_bulletmode.xml) | `0` | Sets active bulletmode. |
| [`g_bulletmodeDebug`](../doc/cvar/cvar.g_bulletmodeDebug.xml) | `0` | Controls the flags for bulletmode debugging. |
| [`g_bulletmodeReference`](../doc/cvar/cvar.g_bulletmodeReference.xml) | `1` | Sets reference bulletmode for comparison. |
| [`g_bulletmodeTrail`](../doc/cvar/cvar.g_bulletmodeTrail.xml) | `0` | Sets maximum number of bullet trails to render. |
| [`g_campaignFile`](../doc/cvar/cvar.g_campaignFile.xml) | `""` | Sets campaign filename. |
| [`g_censor`](../doc/cvar/cvar.g_censor.xml) | `0` | Turns word-censor feature on or off. |
| [`g_censorPenalty`](../doc/cvar/cvar.g_censorPenalty.xml) | `0` | Controls the flags for censorship penalties. |
| [`g_classChange`](../doc/cvar/cvar.g_classChange.xml) | `0` | Turns friendly corpse class stealing on or off. |
| [`g_complaintlimit`](../doc/cvar/cvar.g_complaintlimit.xml) | `6` | Sets the maximum number of complaints a player can receive per map. |
| [`g_covertops`](../doc/cvar/cvar.g_covertops.xml) | `0` | Controls Covert Ops behavior flags. |
| [`g_covertopsChargeTime`](../doc/cvar/cvar.g_covertopsChargeTime.xml) | `30000` | Sets the class ability recharge time. |
| [`g_damagexp`](../doc/cvar/cvar.g_damagexp.xml) | `0` | Turns on XP for weapons damage awarded based on damage inflicted. |
| [`g_debugAlloc`](../doc/cvar/cvar.g_debugAlloc.xml) | `0` | Turns on debugging of the game's server stack. |
| [`g_debugConstruct`](../doc/cvar/cvar.g_debugConstruct.xml) | `0` | Turns on a smaller charge penalty for constructing. |
| [`g_debugDamage`](../doc/cvar/cvar.g_debugDamage.xml) | `0` | Turns on debug information for inflicted damage. |
| [`g_debugMove`](../doc/cvar/cvar.g_debugMove.xml) | `0` | Turns on debug information for player movement. |
| [`g_debugSkills`](../doc/cvar/cvar.g_debugSkills.xml) | `0` | Turns on debugging of the skills system. |
| [`g_defaultSkills`](../doc/cvar/cvar.g_defaultSkills.xml) | `""` | Default skill loadout for connecting players. |
| [`g_disableComplaints`](../doc/cvar/cvar.g_disableComplaints.xml) | `0` | Turns off friendly death complaints for certain weapons. |
| [`g_dragCorpse`](../doc/cvar/cvar.g_dragCorpse.xml) | `0` | Turns on corpse dragging. |
| [`g_dropAmmo`](../doc/cvar/cvar.g_dropAmmo.xml) | `0` | Turns on ammo crate drops on field ops death. |
| [`g_dropHealth`](../doc/cvar/cvar.g_dropHealth.xml) | `0` | Turns on health pack drops on medic death. |
| [`g_dynamiteTime`](../doc/cvar/cvar.g_dynamiteTime.xml) | `30` | Sets the timer for dynamite in seconds. |
| [`g_enforcemaxlives`](../doc/cvar/cvar.g_enforcemaxlives.xml) | `0` | Turns on player tracking to enforce max lives between connects. |
| [`g_engineerChargeTime`](../doc/cvar/cvar.g_engineerChargeTime.xml) | `30000` | Sets the class ability recharge time. |
| [`g_engineers`](../doc/cvar/cvar.g_engineers.xml) | `0` | Controls Engineer behavior flags. |
| [`g_fastres`](../doc/cvar/cvar.g_fastres.xml) | `0` | Turns on fast player revives. |
| [`g_fear`](../doc/cvar/cvar.g_fear.xml) | `0` | Awards the attacker a kill if the victim uses suicide to escape. |
| [`g_filterBan`](../doc/cvar/cvar.g_filterBan.xml) | `1` | Filters players joining the server. |
| [`g_filtercams`](../doc/cvar/cvar.g_filtercams.xml) | `0` | Removes players from camera views. |
| [`g_fixedPhysics`](../doc/cvar/cvar.g_fixedPhysics.xml) | `1` | Turns physics corrections on or off. |
| [`g_fixedPhysicsFPS`](../doc/cvar/cvar.g_fixedPhysicsFPS.xml) | `125` | Sets the emulated FPS used for fixed physics. |
| [`g_forcerespawn`](../doc/cvar/cvar.g_forcerespawn.xml) | `0` | Forces a player to go into limbo after a specified amount of time. |
| [`g_friendlyFire`](../doc/cvar/cvar.g_friendlyFire.xml) | `1` | Turns on friendly fire damage. |
| [`g_gametype`](../doc/cvar/cvar.g_gametype.xml) | `4` | Sets general mode of gameplay. |
| [`g_glow`](../doc/cvar/cvar.g_glow.xml) | `0` | Makes all players emit a colored glow. |
| [`g_goomba`](../doc/cvar/cvar.g_goomba.xml) | `0` | Turns on damage from above. |
| [`g_gravity`](../doc/cvar/cvar.g_gravity.xml) | `800` | Sets the amount of gravity. |
| [`g_headshot`](../doc/cvar/cvar.g_headshot.xml) | `0` | Controls the flags for headshot beahvior. |
| [`g_healthRechargeTime`](../doc/cvar/cvar.g_healthRechargeTime.xml) | `10000` | Sets time interval between ammo-pack cabinet respawns. |
| [`g_heavyWeaponRestriction`](../doc/cvar/cvar.g_heavyWeaponRestriction.xml) | `100` | Sets a limit of heavy weapons that can be used at once per team. |
| [`g_hitmode`](../doc/cvar/cvar.g_hitmode.xml) | `0` | Sets active hitmode. |
| [`g_hitmodeAntilag`](../doc/cvar/cvar.g_hitmodeAntilag.xml) | `800` | Sets maximum amount of antilag in milliseconds. |
| [`g_hitmodeAntilagLerp`](../doc/cvar/cvar.g_hitmodeAntilagLerp.xml) | `1` | Turns antilag lerping on or off. |
| [`g_hitmodeDebug`](../doc/cvar/cvar.g_hitmodeDebug.xml) | `0` | Controls the flags for hitmode debugging. |
| [`g_hitmodeFat`](../doc/cvar/cvar.g_hitmodeFat.xml) | `0` | Sets increased torso-box size in inches. |
| [`g_hitmodeGhosting`](../doc/cvar/cvar.g_hitmodeGhosting.xml) | `0` | Sets lifetime of hit ghosting in milliseconds. |
| [`g_hitmodeReference`](../doc/cvar/cvar.g_hitmodeReference.xml) | `1` | Sets reference hitmode for comparison. |
| [`g_hitmodeZone`](../doc/cvar/cvar.g_hitmodeZone.xml) | `1` | Sets zone for debugging. |
| [`g_inactivity`](../doc/cvar/cvar.g_inactivity.xml) | `0` | Sets player inactivity limit. |
| [`g_intermissionReadyPercent`](../doc/cvar/cvar.g_intermissionReadyPercent.xml) | `100` | Sets the percentage of 'readied' players needed to end intermission. |
| [`g_intermissionTime`](../doc/cvar/cvar.g_intermissionTime.xml) | `60` | Sets the intermission duration. |
| [`g_ipcomplaintlimit`](../doc/cvar/cvar.g_ipcomplaintlimit.xml) | `3` | Sets maximum number of unique complaints allowed for a player. |
| [`g_kickMessage`](../doc/cvar/cvar.g_kickMessage.xml) | `"You have been kicked for $TIME."` | Sets kick message. |
| [`g_kickTime`](../doc/cvar/cvar.g_kickTime.xml) | `2` | Sets duration to ban kicked players. |
| [`g_killingSpree`](../doc/cvar/cvar.g_killingSpree.xml) | `0` | Sets killing spree mode. |
| [`g_killSpreeLevels`](../doc/cvar/cvar.g_killSpreeLevels.xml) | `"5 10 15 20 25 30"` | Sets killing spree XP milestones. |
| [`g_knifeonly`](../doc/cvar/cvar.g_knifeonly.xml) | `0` | Turns knife-only game mode on or off. |
| [`g_knockback`](../doc/cvar/cvar.g_knockback.xml) | `1000` | Sets knockback effect. |
| [`g_landminetimeout`](../doc/cvar/cvar.g_landminetimeout.xml) | `1` | Controls whether a player's landmines are removed after they disconnect. |
| [`g_levels_battlesense`](../doc/cvar/cvar.g_levels_battlesense.xml) | `"" / "20 50 90 140 200"` | Graduated levels of battlesense XP. |
| [`g_levels_covertops`](../doc/cvar/cvar.g_levels_covertops.xml) | `"" / "20 50 90 140 200"` | Graduated levels of XP. |
| [`g_levels_engineer`](../doc/cvar/cvar.g_levels_engineer.xml) | `"" / "20 50 90 140 200"` | Graduated levels of XP. |
| [`g_levels_fieldops`](../doc/cvar/cvar.g_levels_fieldops.xml) | `"" / "20 50 90 140 200"` | Graduated levels of XP. |
| [`g_levels_lightweapons`](../doc/cvar/cvar.g_levels_lightweapons.xml) | `"" / "20 50 90 140 200"` | Graduated levels of lightweapons XP. |
| [`g_levels_medic`](../doc/cvar/cvar.g_levels_medic.xml) | `"" / "20 50 90 140 200"` | Graduated levels of XP. |
| [`g_levels_soldier`](../doc/cvar/cvar.g_levels_soldier.xml) | `"" / "20 50 90 140 200"` | Graduated levels of XP. |
| [`g_lms_followTeamOnly`](../doc/cvar/cvar.g_lms_followTeamOnly.xml) | `1` | Turns same-team spectator restriction on or off. |
| [`g_lms_lockTeams`](../doc/cvar/cvar.g_lms_lockTeams.xml) | `0` | Turns locked teams during match play on or off. |
| [`g_lms_matchlimit`](../doc/cvar/cvar.g_lms_matchlimit.xml) | `2` | Sets maximum number of matches to play before nextmap. |
| [`g_lms_roundlimit`](../doc/cvar/cvar.g_lms_roundlimit.xml) | `3` | Sets maximum number of rounds to play before match ends. |
| [`g_lms_teamForceBalance`](../doc/cvar/cvar.g_lms_teamForceBalance.xml) | `1` | Turns passive team balancing on or off. |
| [`g_log`](../doc/cvar/cvar.g_log.xml) | `""` | Sets game log output file. |
| [`g_logOptions`](../doc/cvar/cvar.g_logOptions.xml) | `0` | Controls the flags for log options. |
| [`g_logSync`](../doc/cvar/cvar.g_logSync.xml) | `0` | Turns log file sync on or off. |
| [`g_loseSpreeLevels`](../doc/cvar/cvar.g_loseSpreeLevels.xml) | `"10 20 30 0 0 0"` | Sets losing spree XP milestones. |
| [`g_LTChargeTime`](../doc/cvar/cvar.g_LTChargeTime.xml) | `40000` | Sets the class ability recharge time. |
| [`g_mapConfigs`](../doc/cvar/cvar.g_mapConfigs.xml) | `""` | Sets directory for map-specific configuration files. |
| [`g_mapScriptDirectory`](../doc/cvar/cvar.g_mapScriptDirectory.xml) | `""` | Sets directory for alternative mapscripts. |
| [`g_maxGameClients`](../doc/cvar/cvar.g_maxGameClients.xml) | `0` | Sets the maximum number of players that can be in the game at one time. |
| [`g_maxlives`](../doc/cvar/cvar.g_maxlives.xml) | `0` | Sets maximum number of lives for all players. |
| [`g_maxlivesRespawnPenalty`](../doc/cvar/cvar.g_maxlivesRespawnPenalty.xml) | `0` | Sets the penalty for a player after their lives have run out. |
| [`g_medicChargeTime`](../doc/cvar/cvar.g_medicChargeTime.xml) | `45000` | Sets the class ability recharge time. |
| [`g_medics`](../doc/cvar/cvar.g_medics.xml) | `0` | Controls Medic behavior flags. |
| [`g_medicSelfHealDelay`](../doc/cvar/cvar.g_medicSelfHealDelay.xml) | `0` | Sets self-healing delay for in milliseconds. |
| [`g_misc`](../doc/cvar/cvar.g_misc.xml) | `0` | Sets various bitflags. |
| [`g_moverScale`](../doc/cvar/cvar.g_moverScale.xml) | `1.0` | Adjusts the speed of movers. |
| [`g_movespeed`](../doc/cvar/cvar.g_movespeed.xml) | `76` | Sets the movement speed of players. |
| [`g_muteTime`](../doc/cvar/cvar.g_muteTime.xml) | `0` | Specifies how long a mute should last. |
| [`g_noTeamSwitching`](../doc/cvar/cvar.g_noTeamSwitching.xml) | `0` | Turns off team switching during a match. |
| [`g_packDistance`](../doc/cvar/cvar.g_packDistance.xml) | `1` | Sets the multiplier of throw distance for ammo and health packs. |
| [`g_panzerWar`](../doc/cvar/cvar.g_panzerWar.xml) | `0` | Turns panzer-war game mode on or off. |
| [`g_password`](../doc/cvar/cvar.g_password.xml) | `""` | Sets password for connecting clients. |
| [`g_playDead`](../doc/cvar/cvar.g_playDead.xml) | `0` | Allows players to use the play-dead feature. |
| [`g_poisonSyringes`](../doc/cvar/cvar.g_poisonSyringes.xml) | `0` | Turns on the use of poison syringes. |
| [`g_privateMessages`](../doc/cvar/cvar.g_privateMessages.xml) | `0` | Turns on private messaging. |
| [`g_proneDelay`](../doc/cvar/cvar.g_proneDelay.xml) | `0` | Turns extended prone delay on or off. |
| [`g_protestMessage`](../doc/cvar/cvar.g_protestMessage.xml) | `""` | Sets a short footer message for players disconnected as punishment. |
| [`g_redlimbotime`](../doc/cvar/cvar.g_redlimbotime.xml) | `30000` | Sets the time between Axis team respawns. |
| [`g_reflectFriendlyFire`](../doc/cvar/cvar.g_reflectFriendlyFire.xml) | `100` | Sets the percentage of friendly fire to reflect to the attacker. |
| [`g_saveCampaignStats`](../doc/cvar/cvar.g_saveCampaignStats.xml) | `0` | Turns on persistent stats across all the maps in a campaign. |
| [`g_scriptDebug`](../doc/cvar/cvar.g_scriptDebug.xml) | `0` | Turns on debug of map script parsing. |
| [`g_scriptDebugLevel`](../doc/cvar/cvar.g_scriptDebugLevel.xml) | `0` | Turns on script generated debug output. |
| [`g_scriptName`](../doc/cvar/cvar.g_scriptName.xml) | `""` | Sets an alternative mapscript to use on a map. |
| [`g_shortcuts`](../doc/cvar/cvar.g_shortcuts.xml) | `0` | Turns text shortcuts on or off. |
| [`g_shoutcastpassword`](../doc/cvar/cvar.g_shoutcastpassword.xml) | `""` | Sets password for clients to use shoutcasting. |
| [`g_shove`](../doc/cvar/cvar.g_shove.xml) | `0` | Sets player shoving distance. |
| [`g_shoveNoZ`](../doc/cvar/cvar.g_shoveNoZ.xml) | `0` | Turns suppression of Z-axis shoving on or off. |
| [`g_sk5_battle`](../doc/cvar/cvar.g_sk5_battle.xml) | `1` | Controls the flags for 5th-level battle-sense skill. |
| [`g_sk5_cvops`](../doc/cvar/cvar.g_sk5_cvops.xml) | `7` | Controls the flags for 5th-level skill. |
| [`g_sk5_eng`](../doc/cvar/cvar.g_sk5_eng.xml) | `127` | Controls the flags for 5th-level skill. |
| [`g_sk5_fdops`](../doc/cvar/cvar.g_sk5_fdops.xml) | `3` | Controls the flags for 5th-level skill. |
| [`g_sk5_lightweap`](../doc/cvar/cvar.g_sk5_lightweap.xml) | `1` | Controls the flags for 5th-level light-weapons skill. |
| [`g_sk5_medic`](../doc/cvar/cvar.g_sk5_medic.xml) | `243` | Controls the flags for 5th-level skill. |
| [`g_sk5_soldier`](../doc/cvar/cvar.g_sk5_soldier.xml) | `7` | Controls the flags for 5th-level skill. |
| [`g_skills`](../doc/cvar/cvar.g_skills.xml) | `0` | Controls the flags for skills related behavior. |
| [`g_slashKill`](../doc/cvar/cvar.g_slashKill.xml) | `0` | Sets client /kill behavior mode. |
| [`g_smoothClients`](../doc/cvar/cvar.g_smoothClients.xml) | `1` | Turns missed client frames smoothing on or off. |
| [`g_snap`](../doc/cvar/cvar.g_snap.xml) | `7` | Controls the flags for server floating point value snapping. |
| [`g_sniperWar`](../doc/cvar/cvar.g_sniperWar.xml) | `0` | Turns sniper-war game mode on or off. |
| [`g_soldierChargeTime`](../doc/cvar/cvar.g_soldierChargeTime.xml) | `20000` | Sets the class ability recharge time. |
| [`g_soldiers`](../doc/cvar/cvar.g_soldiers.xml) | `0` | Controls Soldier behavior flags. |
| [`g_spawnInvul`](../doc/cvar/cvar.g_spawnInvul.xml) | `3` | Sets spawn invulnerability period for players. |
| [`g_spectator`](../doc/cvar/cvar.g_spectator.xml) | `0` | Controls the flags for spectator actions. |
| [`g_spectatorInactivity`](../doc/cvar/cvar.g_spectatorInactivity.xml) | `0` | Sets spectator inactivity limit. |
| [`g_speed`](../doc/cvar/cvar.g_speed.xml) | `320` | Sets player baseline speed. |
| [`g_teamDamageMinHits`](../doc/cvar/cvar.g_teamDamageMinHits.xml) | `6` | Sets friendly-fire tolerance minimum hits. |
| [`g_teamDamageRestriction`](../doc/cvar/cvar.g_teamDamageRestriction.xml) | `0` | Sets friendly-fire tolerance percentage. |
| [`g_teamForceBalance`](../doc/cvar/cvar.g_teamForceBalance.xml) | `0` | Force team balance. |
| [`g_truePing`](../doc/cvar/cvar.g_truePing.xml) | `1` | Turns true ping calculation on or off. |
| [`g_userAlliedRespawnTime`](../doc/cvar/cvar.g_userAlliedRespawnTime.xml) | `0` | Sets the time between Allied team respawns. |
| [`g_userAxisRespawnTime`](../doc/cvar/cvar.g_userAxisRespawnTime.xml) | `0` | Sets the time between Axis team respawns. |
| [`g_voiceChatsAllowed`](../doc/cvar/cvar.g_voiceChatsAllowed.xml) | `4` | Sets maximum number of voice chats per 30 second period. |
| [`g_vulnerableWeapons`](../doc/cvar/cvar.g_vulnerableWeapons.xml) | `0` | Controls the flags to enable missile-type weapon vulnerability. |
| [`g_warmup`](../doc/cvar/cvar.g_warmup.xml) | `60` | Sets warmup period before match begins. |
| [`g_watermark`](../doc/cvar/cvar.g_watermark.xml) | `"jaymod"` | Sets server watermark used for client display. |
| [`g_watermarkFadeAfter`](../doc/cvar/cvar.g_watermarkFadeAfter.xml) | `60` | Sets amount of time before watermark begins to fade. |
| [`g_watermarkFadeTime`](../doc/cvar/cvar.g_watermarkFadeTime.xml) | `60` | Sets amount of time to fade watermark. |
| [`g_weapons`](../doc/cvar/cvar.g_weapons.xml) | `0` | Controls the flags for various weapons behavior. |
| [`g_wolfrof`](../doc/cvar/cvar.g_wolfrof.xml) | `0` | Controls the original Wolfenstein weapon rate of fire. |
| [`g_xpCap`](../doc/cvar/cvar.g_xpCap.xml) | `0` | Sets XP-limit action. |
| [`g_xpMax`](../doc/cvar/cvar.g_xpMax.xml) | `0` | Sets XP-limit amount. |
| [`g_xpSave`](../doc/cvar/cvar.g_xpSave.xml) | `0` | Turns XP-save feature on or off. |
| [`g_xpSaveTimeout`](../doc/cvar/cvar.g_xpSaveTimeout.xml) | `1h` | Sets XP-save duration. |

### Match cvars

| Cvar | Default | What it does |
| --- | --- | --- |
| [`match_latejoin`](../doc/cvar/cvar.match_latejoin.xml) | `1` | Allows or blocks players joining a match already in progress. |
| [`match_minplayers`](../doc/cvar/cvar.match_minplayers.xml) | `0` | Sets minimum number of players required for match to begin. |
| [`match_mutespecs`](../doc/cvar/cvar.match_mutespecs.xml) | `0` | Controls whether spectators are muted during a match. |
| [`match_readypercent`](../doc/cvar/cvar.match_readypercent.xml) | `100` | Sets percentage of players required to be ready. |
| [`match_timeoutcount`](../doc/cvar/cvar.match_timeoutcount.xml) | `3` | Sets maximum number of times non-referees can pause the match. |
| [`match_timeoutlength`](../doc/cvar/cvar.match_timeoutlength.xml) | `180` | Sets duration of player-timeout. |
| [`match_warmupDamage`](../doc/cvar/cvar.match_warmupDamage.xml) | `1` | Turns damage during warmup on or off. |

### Team and class-limit cvars

| Cvar | Default | What it does |
| --- | --- | --- |
| [`team_maxArtillery`](../doc/cvar/cvar.team_maxArtillery.xml) | `6` | Sets the maximum number of artillery or airstrikes per minute. |
| [`team_maxCovertOps`](../doc/cvar/cvar.team_maxCovertOps.xml) | `-1` | Sets the maximum number of covert-ops per team. |
| [`team_maxEngineers`](../doc/cvar/cvar.team_maxEngineers.xml) | `-1` | Sets the maximum number of engineers per team. |
| [`team_maxFieldOps`](../doc/cvar/cvar.team_maxFieldOps.xml) | `-1` | Sets the maximum number of field-ops per team. |
| [`team_maxFlamers`](../doc/cvar/cvar.team_maxFlamers.xml) | `-1` | Sets the maximum number of flamethrowers per team. |
| [`team_maxGrenLaunchers`](../doc/cvar/cvar.team_maxGrenLaunchers.xml) | `-1` | Sets the maximum number of grenade launchers per team. |
| [`team_maxLandMines`](../doc/cvar/cvar.team_maxLandMines.xml) | `10` | Sets the maximum number of landmines per team. |
| [`team_maxM97s`](../doc/cvar/cvar.team_maxM97s.xml) | `-1` | Sets the maximum number of M97s per team. |
| [`team_maxMedics`](../doc/cvar/cvar.team_maxMedics.xml) | `-1` | Sets the maximum number of medics per team. |
| [`team_maxMG42s`](../doc/cvar/cvar.team_maxMG42s.xml) | `-1` | Sets the maximum number of MG42s per team. |
| [`team_maxMortars`](../doc/cvar/cvar.team_maxMortars.xml) | `-1` | Sets the maximum number of mortars per team. |
| [`team_maxPanzers`](../doc/cvar/cvar.team_maxPanzers.xml) | `-1` | Sets maximum number of panzerfausts per team. |
| [`team_maxplayers`](../doc/cvar/cvar.team_maxplayers.xml) | `0` | Sets maximum number of players per team. |
| [`team_nocontrols`](../doc/cvar/cvar.team_nocontrols.xml) | `1` | Turns arbitrary control of teams on or off. |

### Voting cvars

| Cvar | Default | What it does |
| --- | --- | --- |
| [`vote_allow_balancedteams`](../doc/cvar/cvar.vote_allow_balancedteams.xml) | `1` | Allows or blocks balanced-team votes. |
| [`vote_allow_comp`](../doc/cvar/cvar.vote_allow_comp.xml) | `1` | Allows or blocks competition-settings votes. |
| [`vote_allow_friendlyfire`](../doc/cvar/cvar.vote_allow_friendlyfire.xml) | `1` | Allows or blocks friendly-fire votes. |
| [`vote_allow_gametype`](../doc/cvar/cvar.vote_allow_gametype.xml) | `1` | Allows or blocks gametype votes. |
| [`vote_allow_generic`](../doc/cvar/cvar.vote_allow_generic.xml) | `1` | Allows or blocks generic votes. |
| [`vote_allow_kick`](../doc/cvar/cvar.vote_allow_kick.xml) | `1` | Allows or blocks kick votes. |
| [`vote_allow_map`](../doc/cvar/cvar.vote_allow_map.xml) | `1` | Allows or blocks map votes. |
| [`vote_allow_matchreset`](../doc/cvar/cvar.vote_allow_matchreset.xml) | `1` | Allows or blocks match-reset votes. |
| [`vote_allow_matchrestart`](../doc/cvar/cvar.vote_allow_matchrestart.xml) | `1` | Allows or blocks match-restart votes. |
| [`vote_allow_mutespecs`](../doc/cvar/cvar.vote_allow_mutespecs.xml) | `1` | Allows or blocks votes to mute spectators. |
| [`vote_allow_muting`](../doc/cvar/cvar.vote_allow_muting.xml) | `1` | Allows or blocks muting votes. |
| [`vote_allow_nextmap`](../doc/cvar/cvar.vote_allow_nextmap.xml) | `1` | Allows or blocks next-map votes. |
| [`vote_allow_pub`](../doc/cvar/cvar.vote_allow_pub.xml) | `1` | Allows or blocks public-settings votes. |
| [`vote_allow_referee`](../doc/cvar/cvar.vote_allow_referee.xml) | `0` | Allows or blocks referee votes. |
| [`vote_allow_shuffleteamsxp`](../doc/cvar/cvar.vote_allow_shuffleteamsxp.xml) | `1` | Allows or blocks votes to shuffle teams by XP. |
| [`vote_allow_swapteams`](../doc/cvar/cvar.vote_allow_swapteams.xml) | `1` | Allows or blocks team-swap votes. |
| [`vote_allow_timelimit`](../doc/cvar/cvar.vote_allow_timelimit.xml) | `0` | Allows or blocks timelimit votes. |
| [`vote_allow_warmupdamage`](../doc/cvar/cvar.vote_allow_warmupdamage.xml) | `1` | Allows or blocks warmup-damage votes. |
| [`vote_limit`](../doc/cvar/cvar.vote_limit.xml) | `5` | Sets maximum number of times a vote may be called. |
| [`vote_percent`](../doc/cvar/cvar.vote_percent.xml) | `50` | Sets percentage of votes required for it to pass. |
