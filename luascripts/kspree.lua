-- Killstreak tracker, ported from the Nitmod script onto Jaymod 3.1.0 Lua.
-- Load with: set lua_modules "kspree.lua"
--
-- Nitmod !give / !glow / !laserwar / !disguise are not commands here.
-- Gear is granted with et.AddWeaponToPlayer.
-- Adrenaline is powerup slot 12 (level-time expiry).
-- Stamina is powerup slot 5, held up for the reward window.
-- Disguise sets the covert-ops powerup bits. The nameplate stays the player's own name.
-- A revive does not reset the streak. playsound still expects the sound/blue pack.

modname = "KillSpree Tracker"
version = "1.0"

local PW_INVULNERABLE = 1
local PW_NOFATIGUE = 5
local PW_OPS_DISGUISED = 8
local PW_OPS_CLASS_1 = 9
local PW_OPS_CLASS_2 = 10
local PW_OPS_CLASS_3 = 11
local PW_ADRENALINE = 12

-- Large pool so sprint cannot drain it before the reward timer ends.
local STAMINA_POOL = 50000000
local AMMO_REFRESH_MS = 20000

local time_offset = 0

local spree = {}
local death = {}
local suicidestreak = {}
local playerinfo = {}
local stamtimer = {}
local juggernaut = {}
local ammoRefreshTimer = {}

-- ammo, clip. Later rows win when two weapons share an ammo pool.
local LOADOUT = {
    { et.WP_KNIFE, 0, 1 },
    { et.WP_LUGER, 96, 8 },
    { et.WP_SILENCER, 96, 8 },
    { et.WP_AKIMBO_LUGER, 96, 16 },
    { et.WP_AKIMBO_SILENCEDLUGER, 96, 16 },
    { et.WP_COLT, 96, 8 },
    { et.WP_SILENCED_COLT, 96, 8 },
    { et.WP_AKIMBO_COLT, 96, 16 },
    { et.WP_AKIMBO_SILENCEDCOLT, 96, 16 },
    { et.WP_MP40, 180, 32 },
    { et.WP_THOMPSON, 180, 30 },
    { et.WP_STEN, 180, 32 },
    { et.WP_KAR98, 60, 10 },
    { et.WP_CARBINE, 60, 10 },
    { et.WP_GARAND, 80, 8 },
    { et.WP_GARAND_SCOPE, 80, 8 },
    { et.WP_K43, 60, 10 },
    { et.WP_K43_SCOPE, 60, 10 },
    { et.WP_FG42, 150, 20 },
    { et.WP_FG42SCOPE, 150, 20 },
    { et.WP_GPG40, 8, 1 },
    { et.WP_M7, 8, 1 },
    { et.WP_PANZERFAUST, 20, 1 },
    { et.WP_FLAMETHROWER, 300, 0 },
    { et.WP_MOBILE_MG42, 250, 0 },
    { et.WP_MOBILE_MG42_SET, 250, 0 },
    { et.WP_MORTAR, 24, 1 },
    { et.WP_MORTAR_SET, 24, 1 },
    { et.WP_M97, 40, 6 },
    { et.WP_GRENADE_LAUNCHER, 0, 8 },
    { et.WP_GRENADE_PINEAPPLE, 0, 8 },
    { et.WP_MOLOTOV, 0, 4 },
    { et.WP_SMOKE_MARKER, 0, 4 },
    { et.WP_SMOKE_BOMB, 0, 4 },
    { et.WP_DYNAMITE, 0, 4 },
    { et.WP_PLIERS, 0, 1 },
    { et.WP_LANDMINE, 0, 8 },
    { et.WP_SATCHEL, 0, 1 },
    { et.WP_SATCHEL_DET, 0, 1 },
    { et.WP_POISON_GAS, 0, 2 },
    { et.WP_MEDIC_SYRINGE, 0, 20 },
    { et.WP_POISON_SYRINGE, 0, 10 },
    { et.WP_MEDIC_ADRENALINE, 0, 10 },
    { et.WP_MEDKIT, 0, 1 },
    { et.WP_AMMO, 0, 1 },
    { et.WP_BINOCULARS, 1, 0 },
}

local function level_time()
    return et.trap_Milliseconds() + time_offset
end

local function player_name(clientNum)
    local info = et.trap_GetUserinfo(clientNum)
    local name = et.Info_ValueForKey(info, "name")
    if name == nil or name == "" then
        name = et.gentity_get(clientNum, "pers.netname") or "unknown"
    end
    return string.gsub(name, "\"", "")
end

local function cp(clientNum, text)
    et.trap_SendServerCommand(clientNum, "cp \"" .. text .. "\"\n")
end

local function ensure(player)
    if not playerinfo[player] then
        playerinfo[player] = {
            pendingDeathReward = 0,
            disguiseClass = nil,
            disguiseUntil = 0,
            disguiseReapply = 0,
        }
    end
    return playerinfo[player]
end

local function real_killer(victim, killer)
    if killer == nil or killer == victim then
        return false
    end
    if killer == et.ENTITYNUM_WORLD or killer == et.ENTITYNUM_NONE then
        return false
    end
    if killer < 0 or killer >= et.MAX_CLIENTS then
        return false
    end
    return true
end

function ResetAdrenaline(player)
    et.gentity_set(player, "ps.powerups", PW_ADRENALINE, 0)
end

function ResetStamina(player)
    stamtimer[player] = nil
    et.gentity_set(player, "ps.powerups", PW_NOFATIGUE, 0)
end

function ResetGodmode(player)
    et.gentity_set(player, "ps.powerups", PW_INVULNERABLE, 0)
end

function HealthBoost(player, amount)
    local hp = tonumber(et.gentity_get(player, "health")) or 0
    hp = hp + amount
    et.gentity_set(player, "health", hp)
    et.gentity_set(player, "ps.stats", et.STAT_HEALTH, hp)
end

function EnableAdrenaline(player, duration)
    et.gentity_set(player, "ps.powerups", PW_ADRENALINE, level_time() + (duration * 1000))
end

function EnableStamina(player, duration)
    stamtimer[player] = level_time() + (duration * 1000)
    et.gentity_set(player, "ps.powerups", PW_NOFATIGUE, STAMINA_POOL)
end

function EnableGodmode(player, duration)
    et.gentity_set(player, "ps.powerups", PW_INVULNERABLE, level_time() + (duration * 1000))
end

local function grant_weapons(player)
    for i = 1, #LOADOUT do
        local w = LOADOUT[i]
        et.AddWeaponToPlayer(player, w[1], w[2], w[3], 0)
    end
end

local function refill_ammo(player)
    for i = 1, #LOADOUT do
        local w = LOADOUT[i]
        if et.COM_BitCheck(player, w[1]) == 1 then
            et.AddWeaponToPlayer(player, w[1], w[2], w[3], 0)
        end
    end
end

local function apply_disguise(player, class)
    local class1, class2, class3 = 0, 0, 0
    if class == 1 then
        class1 = 1
    elseif class == 2 then
        class2 = 2
    elseif class == 3 then
        class1 = 1
        class2 = 2
    end
    et.gentity_set(player, "ps.powerups", PW_OPS_DISGUISED, 1)
    et.gentity_set(player, "ps.powerups", PW_OPS_CLASS_1, class1)
    et.gentity_set(player, "ps.powerups", PW_OPS_CLASS_2, class2)
    et.gentity_set(player, "ps.powerups", PW_OPS_CLASS_3, class3)
end

local function clear_disguise(player)
    local info = playerinfo[player]
    if info then
        info.disguiseClass = nil
        info.disguiseUntil = 0
        info.disguiseReapply = 0
    end
    et.gentity_set(player, "ps.powerups", PW_OPS_DISGUISED, 0)
    et.gentity_set(player, "ps.powerups", PW_OPS_CLASS_1, 0)
    et.gentity_set(player, "ps.powerups", PW_OPS_CLASS_2, 0)
    et.gentity_set(player, "ps.powerups", PW_OPS_CLASS_3, 0)
end

local function playsound(path)
    et.trap_SendConsoleCommand(et.EXEC_NOW, "playsound " .. path .. "\n")
end

local death_rewards = {
    [-6] = function(player)
        HealthBoost(player, 50)
        cp(player, "^5Losing streak! You got a small HP boost.")
    end,
    [-9] = function(player)
        HealthBoost(player, 50)
        EnableStamina(player, 30)
        cp(player, "^1Hang in there! ^6+50HP ^8& ^1Rapid ^8Stamina regen.")
    end,
    [-12] = function(player)
        HealthBoost(player, 50)
        EnableStamina(player, 60)
        cp(player, "^1Rapid ^8Stamina regen. and +50 HP to help you bounce back!")
    end,
    [-15] = function(player)
        local info = ensure(player)
        info.disguiseClass = math.random(0, 3)
        info.disguiseUntil = level_time() + 20000
        info.disguiseReapply = 0
        apply_disguise(player, info.disguiseClass)
        grant_weapons(player)
        HealthBoost(player, 200)
        EnableStamina(player, 120)
        EnableGodmode(player, 20)
        cp(player, "^8You suck...... here: ^yGear^8, ^6stamina regen^8, ^7Temp ^1Godmode^8 and ^6+200HP!")
        et.trap_SendServerCommand(-1, "chat \"^3" .. player_name(player) .. " ^8is getting ^1Shit on! ^8Be gentle...\"\n")
        playsound("sound/blue/Taunts/taunt07.wav")
    end,
}

local spree_names = {
    [5] = "Health Boost",
    [15] = "Dominator",
    [20] = "Adrenaline Boost",
    [25] = "Tryhard",
    [30] = "Max Gear",
    [50] = "Juggernaut",
    [100] = "Jug Refresh",
    [200] = "Jug God",
}

local kill_rewards = {
    [5] = function(player)
        HealthBoost(player, 50)
        cp(player, "^3Reward: ^6+50HP^8!")
    end,
    [15] = function(player)
        HealthBoost(player, 75)
        cp(player, "^3Reward: ^6+75HP^8!")
    end,
    [20] = function(player)
        HealthBoost(player, 100)
        EnableAdrenaline(player, 20)
        cp(player, "^3Reward: ^6+100HP ^8& ^120s Adrenaline-Boost^8!")
    end,
    [25] = function(player)
        HealthBoost(player, 125)
        EnableAdrenaline(player, 30)
        cp(player, "^3Reward: ^6+125HP^8 ^8& ^130s Adrenaline-Boost^8!")
    end,
    [30] = function(player)
        grant_weapons(player)
        HealthBoost(player, 150)
        EnableAdrenaline(player, 60)
        EnableGodmode(player, 20)
        ammoRefreshTimer[player] = et.trap_Milliseconds()
        cp(player, "^3Reward: ^yFull Loadout^8, ^7Temp ^1Godmode^8, ^1+150HP ^8&  ^160s Adrenaline-Boost^8!")
    end,
    [50] = function(player)
        HealthBoost(player, 3000)
        EnableAdrenaline(player, 180)
        juggernaut[player] = true
        cp(-1, "^1JUGGERNAUT UNLEASHED!! ^7" .. player_name(player) .. " ^1is unstoppable!")
        playsound("sound/blue/Music1/music03.wav")
    end,
    [100] = function(player)
        HealthBoost(player, 2000)
        EnableAdrenaline(player, 300)
        cp(-1, "^1The Juggernaut ^7" .. player_name(player) .. " ^1is at ^4100 KILLS^1! ^8KILL IT NOW!!")
        playsound("sound/blue/Movie/movie08.wav")
    end,
    [200] = function(player)
        HealthBoost(player, 3000)
        EnableAdrenaline(player, 600)
        cp(-1, "^1Are you ^8f***ing ^1kidding me?! ^7200 KILLS^1!?!")
        playsound("sound/blue/Movie/movie08.wav")
    end,
}

function et_InitGame(levelTime, randomSeed, restart)
    et.RegisterModname(modname .. " " .. version)
    et.G_Print("Loaded kspree.lua\n")
    spree = {}
    death = {}
    suicidestreak = {}
    playerinfo = {}
    stamtimer = {}
    juggernaut = {}
    ammoRefreshTimer = {}
    math.randomseed(os.time())
    time_offset = levelTime - et.trap_Milliseconds()
end

function et_ClientSpawn(player, revived)
    if revived == 1 then
        return
    end
    local team = et.gentity_get(player, "sess.sessionTeam")
    if team == et.TEAM_SPECTATOR then
        return
    end

    local pending = 0
    if playerinfo[player] then
        pending = playerinfo[player].pendingDeathReward or 0
    end

    spree[player] = 0
    playerinfo[player] = {
        pendingDeathReward = 0,
        disguiseClass = nil,
        disguiseUntil = 0,
        disguiseReapply = 0,
    }

    if pending ~= 0 and death_rewards[pending] then
        death_rewards[pending](player)
    end
end

function et_ClientDisconnect(player)
    spree[player] = nil
    death[player] = nil
    suicidestreak[player] = nil
    stamtimer[player] = nil
    juggernaut[player] = nil
    ammoRefreshTimer[player] = nil
    playerinfo[player] = nil
end

function et_Obituary(victim, killer, meansOfDeath)
    if killer == et.ENTITYNUM_WORLD or killer == victim then
        suicidestreak[victim] = (suicidestreak[victim] or 0) + 1
        local streak = ((suicidestreak[victim] - 1) % 3) + 1
        local messages = {
            [1] = "^1Love your life!",
            [2] = "^1Suicide is a sin!",
            [3] = "^1Goodbye cruel world!!!",
        }
        cp(victim, messages[streak])
    end

    if juggernaut[victim] then
        juggernaut[victim] = nil
        cp(-1, "^1Juggernaut ^3" .. player_name(victim) .. " ^8has been ^1slain^8!")
    end

    spree[victim] = 0
    ammoRefreshTimer[victim] = nil
    clear_disguise(victim)
    ResetGodmode(victim)
    ResetAdrenaline(victim)
    ResetStamina(victim)

    local info = ensure(victim)
    if real_killer(victim, killer) then
        death[victim] = (death[victim] or 0) - 1
        local thresholds = { -15, -12, -9, -6 }
        info.pendingDeathReward = 0
        for _, threshold in ipairs(thresholds) do
            if death[victim] == threshold then
                info.pendingDeathReward = threshold
                break
            end
        end

        spree[killer] = (spree[killer] or 0) + 1
        death[killer] = 0
        local spree_count = spree[killer]
        if kill_rewards[spree_count] then
            kill_rewards[spree_count](killer)
            local spree_name = spree_names[spree_count] or tostring(spree_count)
            et.trap_SendServerCommand(-1, "cpm \"^1KillingSpree Reward ^2Activated: ^3" .. spree_name .. " ^2by ^7" .. player_name(killer) .. "\"\n")
        end
    else
        info.pendingDeathReward = 0
    end
end

function et_RunFrame(levelTime)
    local now = level_time()
    local now_ms = et.trap_Milliseconds()
    local maxClients = tonumber(et.trap_Cvar_Get("sv_maxclients")) or et.MAX_CLIENTS

    for i = 0, maxClients - 1 do
        if stamtimer[i] and now >= stamtimer[i] then
            ResetStamina(i)
        end

        local info = playerinfo[i]
        if info and info.disguiseUntil and info.disguiseUntil > 0 then
            if now >= info.disguiseUntil then
                clear_disguise(i)
            elseif now >= (info.disguiseReapply or 0) then
                apply_disguise(i, info.disguiseClass or 0)
                info.disguiseReapply = now + 1000
            end
        end

        if ammoRefreshTimer[i] and et.gentity_get(i, "inuse") == 1 then
            if now_ms - ammoRefreshTimer[i] >= AMMO_REFRESH_MS then
                refill_ammo(i)
                ammoRefreshTimer[i] = now_ms
            end
        end
    end
end
