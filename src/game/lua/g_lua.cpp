#include <bgame/impl.h>
#include <game/lua/g_lua.h>

#ifdef FEATURE_LUA

lua_vm_t *lVM[LUA_NUM_VM];

vmCvar_t lua_modules;
vmCvar_t lua_allowedModules;
vmCvar_t lua_reportWebhook;

static int lua_modules_modcount = -1;
static int lua_allowed_modcount = -1;
static qboolean lua_ready = qfalse;

static void G_Lua_ClearVMs( void ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        lVM[i] = NULL;
    }
}

static void G_Lua_FreeVM( lua_vm_t *vm ) {
    if ( !vm ) {
        return;
    }
    if ( vm->L ) {
        lua_close( vm->L );
        vm->L = NULL;
    }
    if ( vm->code ) {
        free( vm->code );
        vm->code = NULL;
    }
    free( vm );
}

static qboolean G_Lua_Allowed( const char *signature ) {
    char allowed[MAX_CVAR_VALUE_STRING];
    char upper[41];
    int i;

    if ( !lua_allowedModules.string[0] ) {
        return qtrue;
    }
    if ( !signature || !signature[0] ) {
        return qfalse;
    }

    Q_strncpyz( allowed, lua_allowedModules.string, sizeof( allowed ) );
    Q_strupr( allowed );
    Q_strncpyz( upper, signature, sizeof( upper ) );
    Q_strupr( upper );
    return strstr( allowed, upper ) ? qtrue : qfalse;
}

static qboolean G_Lua_LoadFile( const char *name, char **out, int *outLen ) {
    fileHandle_t f;
    int len;
    char *buf;
    const char *paths[4];
    int p;

    paths[0] = name;
    paths[1] = va( "luascripts/%s", name );
    paths[2] = va( "lua/%s", name );
    paths[3] = NULL;

    for ( p = 0; paths[p]; p++ ) {
        len = trap_FS_FOpenFile( paths[p], &f, FS_READ );
        if ( len > 0 && f ) {
            if ( len > LUA_MAX_FSIZE ) {
                G_Printf( "Lua: %s is too large (%d bytes)\n", paths[p], len );
                trap_FS_FCloseFile( f );
                return qfalse;
            }
            buf = (char *)malloc( (size_t)len + 1 );
            if ( !buf ) {
                trap_FS_FCloseFile( f );
                return qfalse;
            }
            trap_FS_Read( buf, len, f );
            trap_FS_FCloseFile( f );
            buf[len] = '\0';
            *out = buf;
            *outLen = len;
            return qtrue;
        }
        if ( f ) {
            trap_FS_FCloseFile( f );
        }
    }
    return qfalse;
}

static qboolean G_Lua_StartVM( lua_vm_t *vm ) {
    int err;

    vm->L = luaL_newstate();
    if ( !vm->L ) {
        G_Printf( "Lua: luaL_newstate failed for %s\n", vm->file_name );
        return qfalse;
    }

    luaL_openlibs( vm->L );
    G_Lua_RegisterEtLib( vm->L );

    err = luaL_loadbuffer( vm->L, vm->code, (size_t)vm->code_size, vm->file_name );
    if ( err ) {
        G_Printf( "Lua: load error in %s: %s\n", vm->file_name, lua_tostring( vm->L, -1 ) );
        lua_pop( vm->L, 1 );
        return qfalse;
    }

    err = lua_pcall( vm->L, 0, 0, 0 );
    if ( err ) {
        G_Printf( "Lua: runtime error in %s: %s\n", vm->file_name, lua_tostring( vm->L, -1 ) );
        lua_pop( vm->L, 1 );
        return qfalse;
    }

    return qtrue;
}

int G_Lua_et_IPCSend( lua_State *L ) {
    lua_vm_t *from = NULL;
    lua_vm_t *to;
    int slot;
    const char *msg;
    int i;

    slot = luaL_checkint( L, 1 );
    msg = luaL_checkstring( L, 2 );
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( lVM[i] && lVM[i]->L == L ) {
            from = lVM[i];
            break;
        }
    }
    if ( !from || slot < 0 || slot >= LUA_NUM_VM || !lVM[slot] ) {
        lua_pushinteger( L, 0 );
        return 1;
    }
    to = lVM[slot];
    lua_getglobal( to->L, "et_IPCReceive" );
    if ( !lua_isfunction( to->L, -1 ) ) {
        lua_pop( to->L, 1 );
        lua_pushinteger( L, 0 );
        return 1;
    }
    lua_pushinteger( to->L, from->id );
    lua_pushstring( to->L, msg );
    if ( lua_pcall( to->L, 2, 0, 0 ) != 0 ) {
        G_Printf( "Lua: %s et_IPCReceive: %s\n", to->file_name, lua_tostring( to->L, -1 ) );
        lua_pop( to->L, 1 );
        lua_pushinteger( L, 0 );
        return 1;
    }
    lua_pushinteger( L, 1 );
    return 1;
}

static qboolean G_LuaCall( lua_vm_t *vm, const char *func, int nargs, int nresults ) {
    int err;

    err = lua_pcall( vm->L, nargs, nresults, 0 );
    if ( err ) {
        G_Printf( "Lua: %s %s: %s\n", vm->file_name, func, lua_tostring( vm->L, -1 ) );
        lua_pop( vm->L, 1 );
        return qfalse;
    }
    return qtrue;
}

static qboolean G_LuaGetNamedFunction( lua_vm_t *vm, const char *name ) {
    lua_getglobal( vm->L, name );
    if ( !lua_isfunction( vm->L, -1 ) ) {
        lua_pop( vm->L, 1 );
        return qfalse;
    }
    return qtrue;
}

void G_LuaRegisterCvars( void ) {
    /* registered via gameCvarTable */
}

qboolean G_LuaCvarsChanged( void ) {
    if ( !lua_ready ) {
        return qfalse;
    }
    if ( lua_modules.modificationCount != lua_modules_modcount
         || lua_allowedModules.modificationCount != lua_allowed_modcount ) {
        lua_modules_modcount = lua_modules.modificationCount;
        lua_allowed_modcount = lua_allowedModules.modificationCount;
        return qtrue;
    }
    return qfalse;
}

qboolean G_LuaInit( void ) {
    char buff[MAX_CVAR_VALUE_STRING];
    char *crt;
    int i, len, num = 0;

    G_LuaShutdown();
    G_Lua_ClearVMs();
    lua_ready = qtrue;

    lua_modules_modcount = lua_modules.modificationCount;
    lua_allowed_modcount = lua_allowedModules.modificationCount;

    if ( !lua_modules.string[0] ) {
        G_Printf( "Lua: no modules in lua_modules\n" );
        return qtrue;
    }

    Q_strncpyz( buff, lua_modules.string, sizeof( buff ) );
    len = (int)strlen( buff );
    crt = buff;

    for ( i = 0; i <= len && num < LUA_NUM_VM; i++ ) {
        if ( buff[i] == ' ' || buff[i] == ',' || buff[i] == ';' || buff[i] == '\0' ) {
            buff[i] = '\0';
            if ( crt[0] ) {
                lua_vm_t *vm;
                char *code = NULL;
                int flen = 0;
                const char *sig;

                if ( !G_Lua_LoadFile( crt, &code, &flen ) ) {
                    G_Printf( "Lua: could not read %s\n", crt );
                    crt = buff + i + 1;
                    continue;
                }

                sig = G_SHA1( code );
                if ( !G_Lua_Allowed( sig ) ) {
                    G_Printf( "Lua: %s rejected (sha1 %s not in lua_allowedModules)\n", crt, sig );
                    free( code );
                    crt = buff + i + 1;
                    continue;
                }

                vm = (lua_vm_t *)calloc( 1, sizeof( lua_vm_t ) );
                if ( !vm ) {
                    free( code );
                    crt = buff + i + 1;
                    continue;
                }

                vm->id = num;
                vm->code = code;
                vm->code_size = flen;
                Q_strncpyz( vm->file_name, crt, sizeof( vm->file_name ) );
                Q_strncpyz( vm->mod_signature, sig, sizeof( vm->mod_signature ) );
                Q_strncpyz( vm->mod_name, crt, sizeof( vm->mod_name ) );

                if ( !G_Lua_StartVM( vm ) ) {
                    G_Lua_FreeVM( vm );
                    crt = buff + i + 1;
                    continue;
                }

                lVM[num++] = vm;
                G_Printf( "Lua: loaded %s (sha1 %s)\n", crt, sig );
            }
            crt = buff + i + 1;
        }
    }

    G_Printf( "Lua: %d module(s) loaded\n", num );
    return qtrue;
}

void G_LuaShutdown( void ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( lVM[i] ) {
            if ( lVM[i]->L && G_LuaGetNamedFunction( lVM[i], "et_Quit" ) ) {
                G_LuaCall( lVM[i], "et_Quit", 0, 0 );
            }
            G_Lua_FreeVM( lVM[i] );
            lVM[i] = NULL;
        }
    }
}

void G_LuaRestart( void ) {
    G_Printf( "Lua: restarting modules\n" );
    G_LuaInit();
}

void G_LuaStatus( gentity_t *ent ) {
    int i, cnt = 0;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( lVM[i] ) {
            cnt++;
        }
    }
    if ( ent ) {
        trap_SendServerCommand( ent - g_entities, va( "print \"Lua modules: %d\n\"", cnt ) );
        for ( i = 0; i < LUA_NUM_VM; i++ ) {
            if ( lVM[i] ) {
                trap_SendServerCommand( ent - g_entities,
                    va( "print \"  [%d] %s (%s)\n\"", i, lVM[i]->mod_name, lVM[i]->file_name ) );
            }
        }
    } else {
        G_Printf( "Lua modules: %d\n", cnt );
        for ( i = 0; i < LUA_NUM_VM; i++ ) {
            if ( lVM[i] ) {
                G_Printf( "  [%d] %s (%s)\n", i, lVM[i]->mod_name, lVM[i]->file_name );
            }
        }
    }
}

void G_LuaHook_InitGame( int levelTime, int randomSeed, int restart ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_InitGame" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, levelTime );
        lua_pushinteger( lVM[i]->L, randomSeed );
        lua_pushinteger( lVM[i]->L, restart );
        G_LuaCall( lVM[i], "et_InitGame", 3, 0 );
    }
}

void G_LuaHook_ShutdownGame( int restart ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_ShutdownGame" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, restart );
        G_LuaCall( lVM[i], "et_ShutdownGame", 1, 0 );
    }
}

void G_LuaHook_RunFrame( int levelTime ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_RunFrame" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, levelTime );
        G_LuaCall( lVM[i], "et_RunFrame", 1, 0 );
    }
}

qboolean G_LuaHook_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot, char *reason, int reasonSize ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_ClientConnect" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, clientNum );
        lua_pushinteger( lVM[i]->L, firstTime ? 1 : 0 );
        lua_pushinteger( lVM[i]->L, isBot ? 1 : 0 );
        if ( !G_LuaCall( lVM[i], "et_ClientConnect", 3, 1 ) ) {
            continue;
        }
        if ( lua_isstring( lVM[i]->L, -1 ) ) {
            Q_strncpyz( reason, lua_tostring( lVM[i]->L, -1 ), reasonSize );
            lua_pop( lVM[i]->L, 1 );
            return qtrue;
        }
        lua_pop( lVM[i]->L, 1 );
    }
    return qfalse;
}

void G_LuaHook_ClientDisconnect( int clientNum ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_ClientDisconnect" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, clientNum );
        G_LuaCall( lVM[i], "et_ClientDisconnect", 1, 0 );
    }
}

void G_LuaHook_ClientBegin( int clientNum ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_ClientBegin" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, clientNum );
        G_LuaCall( lVM[i], "et_ClientBegin", 1, 0 );
    }
}

void G_LuaHook_ClientUserinfoChanged( int clientNum ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_ClientUserinfoChanged" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, clientNum );
        G_LuaCall( lVM[i], "et_ClientUserinfoChanged", 1, 0 );
    }
}

void G_LuaHook_ClientSpawn( int clientNum, qboolean revived, qboolean teamChange, qboolean restoreHealth ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_ClientSpawn" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, clientNum );
        lua_pushinteger( lVM[i]->L, revived ? 1 : 0 );
        lua_pushinteger( lVM[i]->L, teamChange ? 1 : 0 );
        lua_pushinteger( lVM[i]->L, restoreHealth ? 1 : 0 );
        G_LuaCall( lVM[i], "et_ClientSpawn", 4, 0 );
    }
}

qboolean G_LuaHook_ClientCommand( int clientNum, const char *command ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_ClientCommand" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, clientNum );
        lua_pushstring( lVM[i]->L, command );
        if ( !G_LuaCall( lVM[i], "et_ClientCommand", 2, 1 ) ) {
            continue;
        }
        if ( lua_tointeger( lVM[i]->L, -1 ) == 1 ) {
            lua_pop( lVM[i]->L, 1 );
            return qtrue;
        }
        lua_pop( lVM[i]->L, 1 );
    }
    return qfalse;
}

qboolean G_LuaHook_ConsoleCommand( const char *command ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_ConsoleCommand" ) ) {
            continue;
        }
        if ( !G_LuaCall( lVM[i], "et_ConsoleCommand", 0, 1 ) ) {
            continue;
        }
        if ( lua_tointeger( lVM[i]->L, -1 ) == 1 ) {
            lua_pop( lVM[i]->L, 1 );
            return qtrue;
        }
        lua_pop( lVM[i]->L, 1 );
    }
    (void)command;
    return qfalse;
}

void G_LuaHook_Obituary( int victim, int killer, int meansOfDeath ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_Obituary" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, victim );
        lua_pushinteger( lVM[i]->L, killer );
        lua_pushinteger( lVM[i]->L, meansOfDeath );
        G_LuaCall( lVM[i], "et_Obituary", 3, 0 );
    }
}

void G_LuaHook_Damage( int target, int attacker, int damage, int dflags, int mod ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_Damage" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, target );
        lua_pushinteger( lVM[i]->L, attacker );
        lua_pushinteger( lVM[i]->L, damage );
        lua_pushinteger( lVM[i]->L, dflags );
        lua_pushinteger( lVM[i]->L, mod );
        G_LuaCall( lVM[i], "et_Damage", 5, 0 );
    }
}

void G_LuaHook_ClientThink( int clientNum ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_ClientThink" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, clientNum );
        G_LuaCall( lVM[i], "et_ClientThink", 1, 0 );
    }
}

void G_LuaHook_WeaponFire( int clientNum, int weapon ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( !lVM[i] || !G_LuaGetNamedFunction( lVM[i], "et_WeaponFire" ) ) {
            continue;
        }
        lua_pushinteger( lVM[i]->L, clientNum );
        lua_pushinteger( lVM[i]->L, weapon );
        G_LuaCall( lVM[i], "et_WeaponFire", 2, 0 );
    }
}

#endif
