#ifndef GAME_G_LUA_H
#define GAME_G_LUA_H

/*
 * ET: Legacy–compatible Lua host for Jaymod.
 *
 * Implements the published Legacy/ETPub Lua API against Jaymod types.
 * This is not a copy of ET: Legacy's GPLv3 g_lua.c.
 */

#ifdef FEATURE_LUA

#define LUA_NUM_VM     16
#define LUA_MAX_FSIZE  (1024 * 1024)

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

struct lua_vm_s {
    int         id;
    char        file_name[MAX_QPATH];
    char        mod_name[MAX_CVAR_VALUE_STRING];
    char        mod_signature[41];
    char       *code;
    int         code_size;
    lua_State  *L;
};

typedef struct lua_vm_s lua_vm_t;

extern lua_vm_t *lVM[LUA_NUM_VM];

extern vmCvar_t lua_modules;
extern vmCvar_t lua_allowedModules;
extern vmCvar_t lua_reportWebhook;

qboolean G_LuaInit( void );
void     G_LuaShutdown( void );
void     G_LuaRestart( void );
void     G_LuaStatus( gentity_t *ent );
void     G_LuaRegisterCvars( void );
qboolean G_LuaCvarsChanged( void );

void G_LuaHook_InitGame( int levelTime, int randomSeed, int restart );
void G_LuaHook_ShutdownGame( int restart );
void G_LuaHook_RunFrame( int levelTime );
qboolean G_LuaHook_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot, char *reason, int reasonSize );
void G_LuaHook_ClientDisconnect( int clientNum );
void G_LuaHook_ClientBegin( int clientNum );
void G_LuaHook_ClientUserinfoChanged( int clientNum );
void G_LuaHook_ClientSpawn( int clientNum, qboolean revived, qboolean teamChange, qboolean restoreHealth );
qboolean G_LuaHook_ClientCommand( int clientNum, const char *command );
qboolean G_LuaHook_ConsoleCommand( const char *command );
void G_LuaHook_Obituary( int victim, int killer, int meansOfDeath );
void G_LuaHook_Damage( int target, int attacker, int damage, int dflags, int mod );
void G_LuaHook_ClientThink( int clientNum );
void G_LuaHook_WeaponFire( int clientNum, int weapon );

void G_Lua_RegisterEtLib( lua_State *L );
void G_Lua_RegisterFields( lua_State *L );

int G_Lua_gentity_get( lua_State *L );
int G_Lua_gentity_set( lua_State *L );
int G_Lua_et_IPCSend( lua_State *L );

const char *G_SHA1( const char *string );

#else

#define G_LuaInit()                         qtrue
#define G_LuaShutdown()                     ((void)0)
#define G_LuaRestart()                      ((void)0)
#define G_LuaHook_InitGame(a,b,c)           ((void)0)
#define G_LuaHook_ShutdownGame(a)           ((void)0)
#define G_LuaHook_RunFrame(a)               ((void)0)
#define G_LuaHook_ClientConnect(a,b,c,d,e)  qfalse
#define G_LuaHook_ClientDisconnect(a)       ((void)0)
#define G_LuaHook_ClientBegin(a)            ((void)0)
#define G_LuaHook_ClientUserinfoChanged(a)  ((void)0)
#define G_LuaHook_ClientSpawn(a,b,c,d)      ((void)0)
#define G_LuaHook_ClientCommand(a,b)        qfalse
#define G_LuaHook_ConsoleCommand(a)         qfalse
#define G_LuaHook_Obituary(a,b,c)           ((void)0)
#define G_LuaHook_Damage(a,b,c,d,e)         ((void)0)
#define G_LuaHook_ClientThink(a)            ((void)0)
#define G_LuaHook_WeaponFire(a,b)           ((void)0)
#define G_LuaCvarsChanged()                 qfalse

#endif

#endif
