#include <bgame/impl.h>
#include <game/lua/g_lua.h>

#ifdef FEATURE_LUA

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#endif

static lua_vm_t *G_LuaGetVM( lua_State *L ) {
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( lVM[i] && lVM[i]->L == L ) {
            return lVM[i];
        }
    }
    return NULL;
}

static int _et_RegisterModname( lua_State *L ) {
    lua_vm_t *vm = G_LuaGetVM( L );
    const char *name = luaL_checkstring( L, 1 );
    if ( vm ) {
        Q_strncpyz( vm->mod_name, name, sizeof( vm->mod_name ) );
    }
    return 0;
}

static int _et_FindSelf( lua_State *L ) {
    lua_vm_t *vm = G_LuaGetVM( L );
    lua_pushinteger( L, vm ? vm->id : -1 );
    return 1;
}

static int _et_FindMod( lua_State *L ) {
    int slot = luaL_checkint( L, 1 );
    if ( slot < 0 || slot >= LUA_NUM_VM || !lVM[slot] ) {
        lua_pushnil( L );
        lua_pushnil( L );
        return 2;
    }
    lua_pushstring( L, lVM[slot]->mod_name );
    lua_pushstring( L, lVM[slot]->mod_signature );
    return 2;
}

static int _et_G_Print( lua_State *L ) {
    const char *text = luaL_checkstring( L, 1 );
    trap_Printf( text );
    return 0;
}

static int _et_G_Printf( lua_State *L ) {
    int n = lua_gettop( L );
    if ( n < 1 ) {
        return 0;
    }
    if ( n == 1 ) {
        trap_Printf( luaL_checkstring( L, 1 ) );
        return 0;
    }
    lua_getglobal( L, "string" );
    lua_getfield( L, -1, "format" );
    lua_remove( L, -2 );
    lua_insert( L, 1 );
    if ( lua_pcall( L, n, 1, 0 ) != 0 ) {
        G_Printf( "Lua G_Printf: %s\n", lua_tostring( L, -1 ) );
        return 0;
    }
    trap_Printf( lua_tostring( L, -1 ) );
    return 0;
}

static int _et_G_LogPrint( lua_State *L ) {
    const char *text = luaL_checkstring( L, 1 );
    G_LogPrintf( "%s", text );
    return 0;
}

static int _et_ConcatArgs( lua_State *L ) {
    int index = luaL_optint( L, 1, 0 );
    lua_pushstring( L, ConcatArgs( index ) );
    return 1;
}

static int _et_trap_Argc( lua_State *L ) {
    lua_pushinteger( L, trap_Argc() );
    return 1;
}

static int _et_trap_Argv( lua_State *L ) {
    char buf[MAX_STRING_CHARS];
    int index = luaL_checkint( L, 1 );
    trap_Argv( index, buf, sizeof( buf ) );
    lua_pushstring( L, buf );
    return 1;
}

static int _et_trap_Cvar_Get( lua_State *L ) {
    char buf[MAX_CVAR_VALUE_STRING];
    const char *name = luaL_checkstring( L, 1 );
    trap_Cvar_VariableStringBuffer( name, buf, sizeof( buf ) );
    lua_pushstring( L, buf );
    return 1;
}

static int _et_trap_Cvar_Set( lua_State *L ) {
    trap_Cvar_Set( luaL_checkstring( L, 1 ), luaL_checkstring( L, 2 ) );
    return 0;
}

static int _et_trap_GetConfigstring( lua_State *L ) {
    char buf[MAX_INFO_STRING];
    trap_GetConfigstring( luaL_checkint( L, 1 ), buf, sizeof( buf ) );
    lua_pushstring( L, buf );
    return 1;
}

static int _et_trap_SetConfigstring( lua_State *L ) {
    trap_SetConfigstring( luaL_checkint( L, 1 ), luaL_checkstring( L, 2 ) );
    return 0;
}

static int _et_trap_SendConsoleCommand( lua_State *L ) {
    trap_SendConsoleCommand( luaL_checkint( L, 1 ), luaL_checkstring( L, 2 ) );
    return 0;
}

static int _et_trap_SendServerCommand( lua_State *L ) {
    trap_SendServerCommand( luaL_checkint( L, 1 ), luaL_checkstring( L, 2 ) );
    return 0;
}

static int _et_trap_DropClient( lua_State *L ) {
    trap_DropClient( luaL_checkint( L, 1 ), luaL_checkstring( L, 2 ), luaL_optint( L, 3, 0 ) );
    return 0;
}

static int _et_ClientNumberFromString( lua_State *L ) {
    int pids[MAX_CLIENTS];
    char name[MAX_NETNAME];
    Q_strncpyz( name, luaL_checkstring( L, 1 ), sizeof( name ) );
    int count = ClientNumbersFromString( name, pids );
    if ( count == 1 ) {
        lua_pushinteger( L, pids[0] );
    } else {
        lua_pushnil( L );
    }
    return 1;
}

static int _et_G_Say( lua_State *L ) {
    int clientNum = luaL_checkint( L, 1 );
    int mode = luaL_checkint( L, 2 );
    const char *text = luaL_checkstring( L, 3 );
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS || !g_entities[clientNum].client ) {
        return 0;
    }
    G_Say( &g_entities[clientNum], NULL, mode, text );
    return 0;
}

static int _et_trap_GetUserinfo( lua_State *L ) {
    char buf[MAX_INFO_STRING];
    trap_GetUserinfo( luaL_checkint( L, 1 ), buf, sizeof( buf ) );
    lua_pushstring( L, buf );
    return 1;
}

static int _et_trap_SetUserinfo( lua_State *L ) {
    trap_SetUserinfo( luaL_checkint( L, 1 ), luaL_checkstring( L, 2 ) );
    return 0;
}

static int _et_ClientUserinfoChanged( lua_State *L ) {
    ClientUserinfoChanged( luaL_checkint( L, 1 ) );
    return 0;
}

static int _et_Info_RemoveKey( lua_State *L ) {
    char buf[MAX_INFO_STRING];
    Q_strncpyz( buf, luaL_checkstring( L, 1 ), sizeof( buf ) );
    Info_RemoveKey( buf, luaL_checkstring( L, 2 ) );
    lua_pushstring( L, buf );
    return 1;
}

static int _et_Info_SetValueForKey( lua_State *L ) {
    char buf[MAX_INFO_STRING];
    Q_strncpyz( buf, luaL_checkstring( L, 1 ), sizeof( buf ) );
    Info_SetValueForKey( buf, luaL_checkstring( L, 2 ), luaL_checkstring( L, 3 ) );
    lua_pushstring( L, buf );
    return 1;
}

static int _et_Info_ValueForKey( lua_State *L ) {
    lua_pushstring( L, Info_ValueForKey( luaL_checkstring( L, 1 ), luaL_checkstring( L, 2 ) ) );
    return 1;
}

static int _et_Q_CleanStr( lua_State *L ) {
    char buf[MAX_STRING_CHARS];
    Q_strncpyz( buf, luaL_checkstring( L, 1 ), sizeof( buf ) );
    lua_pushstring( L, Q_CleanStr( buf ) );
    return 1;
}

static int _et_trap_FS_FOpenFile( lua_State *L ) {
    fileHandle_t f = 0;
    int len = trap_FS_FOpenFile( luaL_checkstring( L, 1 ), &f, (fsMode_t)luaL_checkint( L, 2 ) );
    lua_pushinteger( L, (int)f );
    lua_pushinteger( L, len );
    return 2;
}

static int _et_trap_FS_Read( lua_State *L ) {
    fileHandle_t f = (fileHandle_t)luaL_checkint( L, 1 );
    int count = luaL_checkint( L, 2 );
    char *buf;
    if ( count < 0 ) {
        count = 0;
    }
    if ( count > LUA_MAX_FSIZE ) {
        count = LUA_MAX_FSIZE;
    }
    buf = (char *)malloc( (size_t)count + 1 );
    if ( !buf ) {
        lua_pushstring( L, "" );
        return 1;
    }
    trap_FS_Read( buf, count, f );
    buf[count] = '\0';
    lua_pushlstring( L, buf, (size_t)count );
    free( buf );
    return 1;
}

static int _et_trap_FS_Write( lua_State *L ) {
    size_t len = 0;
    const char *data = luaL_checklstring( L, 1, &len );
    int count = luaL_optint( L, 2, (int)len );
    fileHandle_t f = (fileHandle_t)luaL_checkint( L, 3 );
    if ( count > (int)len ) {
        count = (int)len;
    }
    lua_pushinteger( L, trap_FS_Write( data, count, f ) );
    return 1;
}

static int _et_trap_FS_FCloseFile( lua_State *L ) {
    trap_FS_FCloseFile( (fileHandle_t)luaL_checkint( L, 1 ) );
    return 0;
}

static int _et_trap_FS_Rename( lua_State *L ) {
    trap_FS_Rename( luaL_checkstring( L, 1 ), luaL_checkstring( L, 2 ) );
    return 0;
}

static int _et_trap_FS_GetFileList( lua_State *L ) {
    char listbuf[4096];
    int n, i, pos = 0;
    n = trap_FS_GetFileList( luaL_checkstring( L, 1 ), luaL_optstring( L, 2, "" ), listbuf, sizeof( listbuf ) );
    lua_newtable( L );
    for ( i = 1; i <= n && pos < (int)sizeof( listbuf ); i++ ) {
        lua_pushstring( L, listbuf + pos );
        lua_rawseti( L, -2, i );
        pos += (int)strlen( listbuf + pos ) + 1;
    }
    return 1;
}

static int _et_trap_Milliseconds( lua_State *L ) {
    lua_pushinteger( L, trap_Milliseconds() );
    return 1;
}

#ifdef _WIN32

static wchar_t *G_Lua_Utf8ToWide( const char *src ) {
    int n;
    wchar_t *out;
    if ( !src ) {
        return NULL;
    }
    n = MultiByteToWideChar( CP_UTF8, 0, src, -1, NULL, 0 );
    if ( n <= 0 ) {
        return NULL;
    }
    out = (wchar_t *)malloc( (size_t)n * sizeof( wchar_t ) );
    if ( !out ) {
        return NULL;
    }
    MultiByteToWideChar( CP_UTF8, 0, src, -1, out, n );
    return out;
}

static char *G_Lua_WideToUtf8( const wchar_t *src, int srcLen ) {
    int n;
    char *out;
    n = WideCharToMultiByte( CP_UTF8, 0, src, srcLen, NULL, 0, NULL, NULL );
    if ( n <= 0 ) {
        return NULL;
    }
    out = (char *)malloc( (size_t)n + 1 );
    if ( !out ) {
        return NULL;
    }
    WideCharToMultiByte( CP_UTF8, 0, src, srcLen, out, n, NULL, NULL );
    out[n] = '\0';
    return out;
}

static int _et_httpRequest( lua_State *L ) {
    const char *url = luaL_checkstring( L, 1 );
    const char *method = luaL_optstring( L, 2, "GET" );
    size_t bodyLen = 0;
    const char *body = luaL_optlstring( L, 3, "", &bodyLen );
    wchar_t extraHeaders[2048];
    wchar_t *wurl = NULL;
    URL_COMPONENTS uc;
    wchar_t host[256];
    wchar_t path[2048];
    wchar_t extra[512];
    HINTERNET session = NULL, connect = NULL, request = NULL;
    DWORD status = 0, statusSize = sizeof( status );
    char *response = NULL;
    size_t responseLen = 0;
    int timeoutMs = 4000;
    INTERNET_PORT port;
    BOOL ok;
    wchar_t wmethod[16];

    extraHeaders[0] = 0;
    if ( lua_istable( L, 4 ) ) {
        lua_pushnil( L );
        while ( lua_next( L, 4 ) != 0 ) {
            if ( lua_isstring( L, -2 ) && lua_isstring( L, -1 ) ) {
                wchar_t *wk = G_Lua_Utf8ToWide( lua_tostring( L, -2 ) );
                wchar_t *wv = G_Lua_Utf8ToWide( lua_tostring( L, -1 ) );
                if ( wk && wv ) {
                    wchar_t line[512];
                    _snwprintf( line, 511, L"%s: %s\r\n", wk, wv );
                    line[511] = 0;
                    wcsncat( extraHeaders, line, 2047 - wcslen( extraHeaders ) );
                }
                free( wk );
                free( wv );
            }
            lua_pop( L, 1 );
        }
    }

    wurl = G_Lua_Utf8ToWide( url );
    if ( !wurl ) {
        lua_pushinteger( L, 0 );
        lua_pushstring( L, "bad url" );
        return 2;
    }

    memset( &uc, 0, sizeof( uc ) );
    uc.dwStructSize = sizeof( uc );
    uc.lpszHostName = host;
    uc.dwHostNameLength = 256;
    uc.lpszUrlPath = path;
    uc.dwUrlPathLength = 2048;
    uc.lpszExtraInfo = extra;
    uc.dwExtraInfoLength = 512;

    if ( !WinHttpCrackUrl( wurl, 0, 0, &uc ) ) {
        free( wurl );
        lua_pushinteger( L, 0 );
        lua_pushstring( L, "url parse failed" );
        return 2;
    }

    if ( uc.lpszExtraInfo && extra[0] ) {
        wcsncat( path, extra, 2047 - wcslen( path ) );
    }

    port = uc.nPort ? uc.nPort : ( ( uc.nScheme == INTERNET_SCHEME_HTTPS ) ? 443 : 80 );
    MultiByteToWideChar( CP_UTF8, 0, method, -1, wmethod, 16 );

    session = WinHttpOpen( L"Jaymod-Lua/3.1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );
    if ( !session ) {
        free( wurl );
        lua_pushinteger( L, 0 );
        lua_pushstring( L, "WinHttpOpen failed" );
        return 2;
    }
    WinHttpSetTimeouts( session, timeoutMs, timeoutMs, timeoutMs, timeoutMs );

    connect = WinHttpConnect( session, host, port, 0 );
    if ( !connect ) {
        WinHttpCloseHandle( session );
        free( wurl );
        lua_pushinteger( L, 0 );
        lua_pushstring( L, "connect failed" );
        return 2;
    }

    request = WinHttpOpenRequest( connect, wmethod, path, NULL, WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES,
                                  ( uc.nScheme == INTERNET_SCHEME_HTTPS ) ? WINHTTP_FLAG_SECURE : 0 );
    if ( !request ) {
        WinHttpCloseHandle( connect );
        WinHttpCloseHandle( session );
        free( wurl );
        lua_pushinteger( L, 0 );
        lua_pushstring( L, "open request failed" );
        return 2;
    }

    ok = WinHttpSendRequest( request,
                             extraHeaders[0] ? extraHeaders : WINHTTP_NO_ADDITIONAL_HEADERS,
                             extraHeaders[0] ? (DWORD)-1 : 0,
                             ( bodyLen > 0 ) ? (LPVOID)body : WINHTTP_NO_REQUEST_DATA,
                             (DWORD)bodyLen, (DWORD)bodyLen, 0 );
    if ( ok ) {
        ok = WinHttpReceiveResponse( request, NULL );
    }
    if ( !ok ) {
        WinHttpCloseHandle( request );
        WinHttpCloseHandle( connect );
        WinHttpCloseHandle( session );
        free( wurl );
        lua_pushinteger( L, 0 );
        lua_pushstring( L, "http send/recv failed" );
        return 2;
    }

    WinHttpQueryHeaders( request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                         WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX );

    for ( ;; ) {
        DWORD avail = 0, read = 0;
        char chunk[4096];
        char *nbuf;
        if ( !WinHttpQueryDataAvailable( request, &avail ) || avail == 0 ) {
            break;
        }
        if ( avail > sizeof( chunk ) ) {
            avail = sizeof( chunk );
        }
        if ( !WinHttpReadData( request, chunk, avail, &read ) || read == 0 ) {
            break;
        }
        nbuf = (char *)realloc( response, responseLen + read + 1 );
        if ( !nbuf ) {
            break;
        }
        response = nbuf;
        memcpy( response + responseLen, chunk, read );
        responseLen += read;
        response[responseLen] = '\0';
        if ( responseLen > LUA_MAX_FSIZE ) {
            break;
        }
    }

    lua_pushinteger( L, (int)status );
    lua_pushlstring( L, response ? response : "", responseLen );
    free( response );
    WinHttpCloseHandle( request );
    WinHttpCloseHandle( connect );
    WinHttpCloseHandle( session );
    free( wurl );
    return 2;
}

#else

static int _et_httpRequest( lua_State *L ) {
    lua_pushinteger( L, 0 );
    lua_pushstring( L, "et.httpRequest is only available on Windows" );
    return 2;
}

#endif

static gentity_t *G_Lua_Entity( lua_State *L, int idx, qboolean allowWorld ) {
    int n;
    if ( lua_isnoneornil( L, idx ) ) {
        return NULL;
    }
    n = luaL_checkint( L, idx );
    if ( n < 0 || n >= MAX_GENTITIES ) {
        return NULL;
    }
    if ( !allowWorld && n >= ENTITYNUM_MAX_NORMAL && !g_entities[n].inuse ) {
        return NULL;
    }
    return &g_entities[n];
}

static qboolean G_Lua_ClientOk( int n ) {
    return ( n >= 0 && n < MAX_CLIENTS && g_entities[n].inuse && g_entities[n].client ) ? qtrue : qfalse;
}

static void G_Lua_ReadVec3( lua_State *L, int idx, vec3_t out ) {
    VectorClear( out );
    if ( !lua_istable( L, idx ) ) {
        return;
    }
    lua_rawgeti( L, idx, 1 ); out[0] = (float)lua_tonumber( L, -1 ); lua_pop( L, 1 );
    lua_rawgeti( L, idx, 2 ); out[1] = (float)lua_tonumber( L, -1 ); lua_pop( L, 1 );
    lua_rawgeti( L, idx, 3 ); out[2] = (float)lua_tonumber( L, -1 ); lua_pop( L, 1 );
}

static void G_Lua_PushVec3( lua_State *L, const vec3_t v ) {
    lua_newtable( L );
    lua_pushnumber( L, v[0] ); lua_rawseti( L, -2, 1 );
    lua_pushnumber( L, v[1] ); lua_rawseti( L, -2, 2 );
    lua_pushnumber( L, v[2] ); lua_rawseti( L, -2, 3 );
}

static int _et_G_Damage( lua_State *L ) {
    gentity_t *targ = G_Lua_Entity( L, 1, qtrue );
    gentity_t *inflictor;
    gentity_t *attacker;
    vec3_t dir, point;
    int damage, dflags, mod;
    qboolean haveDir = qfalse;

    if ( !targ ) {
        return 0;
    }
    inflictor = G_Lua_Entity( L, 2, qtrue );
    attacker = G_Lua_Entity( L, 3, qtrue );
    if ( lua_istable( L, 4 ) ) {
        G_Lua_ReadVec3( L, 4, dir );
        G_Lua_ReadVec3( L, 5, point );
        damage = luaL_checkint( L, 6 );
        dflags = luaL_optint( L, 7, 0 );
        mod = luaL_optint( L, 8, MOD_UNKNOWN );
        haveDir = qtrue;
    } else {
        damage = luaL_checkint( L, 4 );
        dflags = luaL_optint( L, 5, 0 );
        mod = luaL_optint( L, 6, MOD_UNKNOWN );
        if ( lua_istable( L, 7 ) ) {
            G_Lua_ReadVec3( L, 7, dir );
            haveDir = qtrue;
        }
        if ( lua_istable( L, 8 ) ) {
            G_Lua_ReadVec3( L, 8, point );
        } else {
            VectorClear( point );
        }
    }
    G_Damage( targ, inflictor, attacker, haveDir ? dir : NULL, point, damage, dflags, mod );
    return 0;
}

static int _et_G_XP_Set( lua_State *L ) {
    int clientNum = luaL_checkint( L, 1 );
    float xp = (float)luaL_checknumber( L, 2 );
    int skill = luaL_checkint( L, 3 );
    if ( !G_Lua_ClientOk( clientNum ) || skill < 0 || skill >= SK_NUM_SKILLS ) {
        lua_pushinteger( L, 0 );
        return 1;
    }
    g_entities[clientNum].client->sess.skillpoints[skill] = xp;
    if ( xp < 0.0f ) {
        g_entities[clientNum].client->sess.skillpoints[skill] = 0.0f;
    }
    G_SetPlayerSkill( g_entities[clientNum].client, (skillType_t)skill );
    G_CalcRank( g_entities[clientNum].client );
    lua_pushinteger( L, 1 );
    return 1;
}

static int _et_G_ResetXP( lua_State *L ) {
    int clientNum = luaL_checkint( L, 1 );
    if ( !G_Lua_ClientOk( clientNum ) ) {
        return 0;
    }
    g_clientObjects[clientNum].xpReset();
    return 0;
}

static int _et_G_AddSkillPoints( lua_State *L ) {
    int entnum = luaL_checkint( L, 1 );
    int skill = luaL_checkint( L, 2 );
    float points = (float)luaL_checknumber( L, 3 );
    if ( !G_Lua_ClientOk( entnum ) || skill < 0 || skill >= SK_NUM_SKILLS ) {
        return 0;
    }
    G_AddSkillPoints( &g_entities[entnum], (skillType_t)skill, points );
    return 0;
}

static int _et_G_LoseSkillPoints( lua_State *L ) {
    int entnum = luaL_checkint( L, 1 );
    int skill = luaL_checkint( L, 2 );
    float points = (float)luaL_checknumber( L, 3 );
    if ( !G_Lua_ClientOk( entnum ) || skill < 0 || skill >= SK_NUM_SKILLS ) {
        return 0;
    }
    G_LoseSkillPoints( &g_entities[entnum], (skillType_t)skill, points );
    return 0;
}

static int _et_G_SoundIndex( lua_State *L ) {
    lua_pushinteger( L, G_SoundIndex( luaL_checkstring( L, 1 ) ) );
    return 1;
}

static int _et_G_ModelIndex( lua_State *L ) {
    char name[MAX_QPATH];
    Q_strncpyz( name, luaL_checkstring( L, 1 ), sizeof( name ) );
    lua_pushinteger( L, G_ModelIndex( name ) );
    return 1;
}

static int _et_G_ShaderIndex( lua_State *L ) {
    char name[MAX_QPATH];
    Q_strncpyz( name, luaL_checkstring( L, 1 ), sizeof( name ) );
    lua_pushinteger( L, G_ShaderIndex( name ) );
    return 1;
}

static int _et_G_Sound( lua_State *L ) {
    gentity_t *ent = G_Lua_Entity( L, 1, qtrue );
    if ( ent ) {
        G_Sound( ent, luaL_checkint( L, 2 ) );
    }
    return 0;
}

static int _et_G_globalSound( lua_State *L ) {
    G_globalSound( luaL_checkstring( L, 1 ) );
    return 0;
}

static int _et_G_ClientSound( lua_State *L ) {
    gentity_t *ent = G_Lua_Entity( L, 1, qfalse );
    if ( !ent || !ent->client ) {
        return 0;
    }
    if ( lua_isnumber( L, 2 ) ) {
        gentity_t *hs = G_TempEntity( ent->client->ps.origin, EV_GLOBAL_CLIENT_SOUND );
        hs->s.teamNum = ent->client - level.clients;
        hs->s.eventParm = luaL_checkint( L, 2 );
    } else {
        G_ClientSound( ent, luaL_checkstring( L, 2 ) );
    }
    return 0;
}

static int _et_MutePlayer( lua_State *L ) {
    int n = luaL_checkint( L, 1 );
    if ( G_Lua_ClientOk( n ) ) {
        G_MutePlayer( &g_entities[n], "lua", luaL_optstring( L, 2, "" ) );
    }
    return 0;
}

static int _et_UnmutePlayer( lua_State *L ) {
    int n = luaL_checkint( L, 1 );
    if ( G_Lua_ClientOk( n ) ) {
        G_UnmutePlayer( &g_entities[n] );
    }
    return 0;
}

static int _et_AddWeaponToPlayer( lua_State *L ) {
    int n = luaL_checkint( L, 1 );
    int weapon = luaL_checkint( L, 2 );
    int ammo = luaL_optint( L, 3, 0 );
    int clip = luaL_optint( L, 4, 0 );
    qboolean setcurrent = luaL_optint( L, 5, 0 ) ? qtrue : qfalse;
    if ( !G_Lua_ClientOk( n ) || weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS ) {
        lua_pushinteger( L, 0 );
        return 1;
    }
    lua_pushinteger( L, AddWeaponToPlayer( g_entities[n].client, (weapon_t)weapon, ammo, clip, setcurrent ) ? 1 : 0 );
    return 1;
}

static int _et_RemoveWeaponFromPlayer( lua_State *L ) {
    int n = luaL_checkint( L, 1 );
    int weapon = luaL_checkint( L, 2 );
    if ( !G_Lua_ClientOk( n ) || weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS ) {
        return 0;
    }
    COM_BitClear( g_entities[n].client->ps.weapons, weapon );
    if ( g_entities[n].client->ps.weapon == weapon ) {
        g_entities[n].client->ps.weapon = WP_NONE;
    }
    return 0;
}

static int _et_GetCurrentWeapon( lua_State *L ) {
    int n = luaL_checkint( L, 1 );
    if ( !G_Lua_ClientOk( n ) ) {
        lua_pushinteger( L, 0 );
        return 1;
    }
    lua_pushinteger( L, g_entities[n].client->ps.weapon );
    return 1;
}

static int _et_G_Spawn( lua_State *L ) {
    gentity_t *ent = G_Spawn();
    lua_pushinteger( L, ent ? (int)( ent - g_entities ) : -1 );
    return 1;
}

static int _et_G_FreeEntity( lua_State *L ) {
    gentity_t *ent = G_Lua_Entity( L, 1, qfalse );
    if ( ent && ent->inuse && ( ent - g_entities ) >= MAX_CLIENTS ) {
        G_FreeEntity( ent );
    }
    return 0;
}

static int _et_G_EntitiesFree( lua_State *L ) {
    int i, n = 0;
    for ( i = MAX_CLIENTS; i < MAX_GENTITIES; i++ ) {
        if ( !g_entities[i].inuse ) {
            n++;
        }
    }
    lua_pushinteger( L, n );
    return 1;
}

static int _et_G_TempEntity( lua_State *L ) {
    vec3_t origin;
    gentity_t *ent;
    G_Lua_ReadVec3( L, 1, origin );
    ent = G_TempEntity( origin, luaL_checkint( L, 2 ) );
    lua_pushinteger( L, ent ? (int)( ent - g_entities ) : -1 );
    return 1;
}

static int _et_G_AddEvent( lua_State *L ) {
    gentity_t *ent = G_Lua_Entity( L, 1, qtrue );
    if ( ent ) {
        G_AddEvent( ent, luaL_checkint( L, 2 ), luaL_optint( L, 3, 0 ) );
    }
    return 0;
}

static int _et_G_SetOrigin( lua_State *L ) {
    gentity_t *ent = G_Lua_Entity( L, 1, qtrue );
    vec3_t origin;
    if ( !ent ) {
        return 0;
    }
    G_Lua_ReadVec3( L, 2, origin );
    G_SetOrigin( ent, origin );
    return 0;
}

static int _et_G_SetAngle( lua_State *L ) {
    gentity_t *ent = G_Lua_Entity( L, 1, qtrue );
    vec3_t angles;
    if ( !ent ) {
        return 0;
    }
    G_Lua_ReadVec3( L, 2, angles );
    G_SetAngle( ent, angles );
    return 0;
}

static int _et_G_SetEntState( lua_State *L ) {
    gentity_t *ent = G_Lua_Entity( L, 1, qtrue );
    if ( ent ) {
        G_SetEntState( ent, (entState_t)luaL_checkint( L, 2 ) );
    }
    return 0;
}

static int _et_trap_LinkEntity( lua_State *L ) {
    gentity_t *ent = G_Lua_Entity( L, 1, qtrue );
    if ( ent ) {
        trap_LinkEntity( ent );
    }
    return 0;
}

static int _et_trap_UnlinkEntity( lua_State *L ) {
    gentity_t *ent = G_Lua_Entity( L, 1, qtrue );
    if ( ent ) {
        trap_UnlinkEntity( ent );
    }
    return 0;
}

static int _et_trap_Trace( lua_State *L ) {
    trace_t tr;
    vec3_t start, mins, maxs, end;
    G_Lua_ReadVec3( L, 1, start );
    G_Lua_ReadVec3( L, 2, mins );
    G_Lua_ReadVec3( L, 3, maxs );
    G_Lua_ReadVec3( L, 4, end );
    trap_Trace( &tr, start, mins, maxs, end, luaL_optint( L, 5, ENTITYNUM_NONE ), luaL_optint( L, 6, MASK_SHOT ) );
    lua_newtable( L );
    lua_pushboolean( L, tr.allsolid ); lua_setfield( L, -2, "allsolid" );
    lua_pushboolean( L, tr.startsolid ); lua_setfield( L, -2, "startsolid" );
    lua_pushnumber( L, tr.fraction ); lua_setfield( L, -2, "fraction" );
    G_Lua_PushVec3( L, tr.endpos ); lua_setfield( L, -2, "endpos" );
    lua_pushinteger( L, tr.surfaceFlags ); lua_setfield( L, -2, "surfaceFlags" );
    lua_pushinteger( L, tr.contents ); lua_setfield( L, -2, "contents" );
    lua_pushinteger( L, tr.entityNum ); lua_setfield( L, -2, "entityNum" );
    G_Lua_PushVec3( L, tr.plane.normal ); lua_setfield( L, -2, "plane" );
    return 1;
}

static int _et_trap_PointContents( lua_State *L ) {
    vec3_t point;
    G_Lua_ReadVec3( L, 1, point );
    lua_pushinteger( L, trap_PointContents( point, luaL_optint( L, 2, ENTITYNUM_NONE ) ) );
    return 1;
}

static int _et_trap_InPVS( lua_State *L ) {
    vec3_t a, b;
    G_Lua_ReadVec3( L, 1, a );
    G_Lua_ReadVec3( L, 2, b );
    lua_pushboolean( L, trap_InPVS( a, b ) );
    return 1;
}

static int _et_trap_EntitiesInBox( lua_State *L ) {
    vec3_t mins, maxs;
    int list[MAX_GENTITIES];
    int n, i;
    G_Lua_ReadVec3( L, 1, mins );
    G_Lua_ReadVec3( L, 2, maxs );
    n = trap_EntitiesInBox( mins, maxs, list, MAX_GENTITIES );
    lua_newtable( L );
    for ( i = 0; i < n; i++ ) {
        lua_pushinteger( L, list[i] );
        lua_rawseti( L, -2, i + 1 );
    }
    return 1;
}

static int _et_isBitSet( lua_State *L ) {
    int bit = luaL_checkint( L, 1 );
    int value = luaL_checkint( L, 2 );
    lua_pushinteger( L, ( bit >= 0 && bit < 32 && ( value & ( 1 << bit ) ) ) ? 1 : 0 );
    return 1;
}

static int _et_COM_BitCheck( lua_State *L ) {
    int n = luaL_checkint( L, 1 );
    int bit = luaL_checkint( L, 2 );
    if ( !G_Lua_ClientOk( n ) ) {
        lua_pushinteger( L, 0 );
        return 1;
    }
    lua_pushinteger( L, COM_BitCheck( g_entities[n].client->ps.weapons, bit ) ? 1 : 0 );
    return 1;
}

static int _et_COM_BitSet( lua_State *L ) {
    int n = luaL_checkint( L, 1 );
    int bit = luaL_checkint( L, 2 );
    if ( G_Lua_ClientOk( n ) ) {
        COM_BitSet( g_entities[n].client->ps.weapons, bit );
    }
    return 0;
}

static int _et_COM_BitClear( lua_State *L ) {
    int n = luaL_checkint( L, 1 );
    int bit = luaL_checkint( L, 2 );
    if ( G_Lua_ClientOk( n ) ) {
        COM_BitClear( g_entities[n].client->ps.weapons, bit );
    }
    return 0;
}

static int _et_G_SHA1( lua_State *L ) {
    lua_pushstring( L, G_SHA1( luaL_checkstring( L, 1 ) ) );
    return 1;
}

static int _et_G_GetLevelTime( lua_State *L ) {
    lua_pushinteger( L, level.time );
    return 1;
}

static int _et_G_GetStartTime( lua_State *L ) {
    lua_pushinteger( L, level.startTime );
    return 1;
}

static int _et_PlayerName( lua_State *L ) {
    int n = luaL_checkint( L, 1 );
    if ( !G_Lua_ClientOk( n ) ) {
        lua_pushnil( L );
        return 1;
    }
    lua_pushstring( L, g_entities[n].client->pers.netname );
    return 1;
}

static int _et_IsConnected( lua_State *L ) {
    int n = luaL_checkint( L, 1 );
    lua_pushinteger( L, ( G_Lua_ClientOk( n ) && g_entities[n].client->pers.connected == CON_CONNECTED ) ? 1 : 0 );
    return 1;
}

static int _et_trap_Cvar_GetInteger( lua_State *L ) {
    lua_pushinteger( L, trap_Cvar_VariableIntegerValue( luaL_checkstring( L, 1 ) ) );
    return 1;
}

static int _et_Q_stricmp( lua_State *L ) {
    lua_pushinteger( L, Q_stricmp( luaL_checkstring( L, 1 ), luaL_checkstring( L, 2 ) ) );
    return 1;
}

static int _et_FindModByName( lua_State *L ) {
    const char *name = luaL_checkstring( L, 1 );
    int i;
    for ( i = 0; i < LUA_NUM_VM; i++ ) {
        if ( lVM[i] && !Q_stricmp( lVM[i]->mod_name, name ) ) {
            lua_pushinteger( L, i );
            return 1;
        }
    }
    lua_pushinteger( L, -1 );
    return 1;
}

static int _et_G_Find( lua_State *L ) {
    gentity_t *from = NULL;
    const char *field = luaL_optstring( L, 2, "classname" );
    const char *match = luaL_checkstring( L, 3 );
    int ofs;
    gentity_t *ent;
    if ( !lua_isnoneornil( L, 1 ) ) {
        int n = luaL_checkint( L, 1 );
        if ( n >= 0 && n < MAX_GENTITIES ) {
            from = &g_entities[n];
        }
    }
    if ( !Q_stricmp( field, "classname" ) ) {
        ofs = FOFS( classname );
    } else if ( !Q_stricmp( field, "targetname" ) ) {
        ofs = FOFS( targetname );
    } else if ( !Q_stricmp( field, "target" ) ) {
        ofs = FOFS( target );
    } else if ( !Q_stricmp( field, "message" ) ) {
        ofs = FOFS( message );
    } else {
        lua_pushnil( L );
        return 1;
    }
    ent = G_Find( from, ofs, match );
    if ( ent ) {
        lua_pushinteger( L, (int)( ent - g_entities ) );
    } else {
        lua_pushnil( L );
    }
    return 1;
}

static int _et_G_UseTargets( lua_State *L ) {
    gentity_t *ent = G_Lua_Entity( L, 1, qtrue );
    gentity_t *activator = G_Lua_Entity( L, 2, qtrue );
    if ( ent ) {
        G_UseTargets( ent, activator );
    }
    return 0;
}

static int _et_AddRemap( lua_State *L ) {
    AddRemap( luaL_checkstring( L, 1 ), luaL_checkstring( L, 2 ), (float)luaL_optnumber( L, 3, 0 ) );
    trap_SetConfigstring( CS_SHADERSTATE, BuildShaderStateConfig() );
    return 0;
}

static int _et_trap_GetServerinfo( lua_State *L ) {
    char buf[MAX_INFO_STRING];
    trap_GetServerinfo( buf, sizeof( buf ) );
    lua_pushstring( L, buf );
    return 1;
}

static int _et_trap_RealTime( lua_State *L ) {
    qtime_t tm;
    trap_RealTime( &tm );
    lua_newtable( L );
    lua_pushinteger( L, tm.tm_sec ); lua_setfield( L, -2, "tm_sec" );
    lua_pushinteger( L, tm.tm_min ); lua_setfield( L, -2, "tm_min" );
    lua_pushinteger( L, tm.tm_hour ); lua_setfield( L, -2, "tm_hour" );
    lua_pushinteger( L, tm.tm_mday ); lua_setfield( L, -2, "tm_mday" );
    lua_pushinteger( L, tm.tm_mon ); lua_setfield( L, -2, "tm_mon" );
    lua_pushinteger( L, tm.tm_year ); lua_setfield( L, -2, "tm_year" );
    lua_pushinteger( L, tm.tm_wday ); lua_setfield( L, -2, "tm_wday" );
    lua_pushinteger( L, tm.tm_yday ); lua_setfield( L, -2, "tm_yday" );
    lua_pushinteger( L, tm.tm_isdst ); lua_setfield( L, -2, "tm_isdst" );
    return 1;
}

#define lua_regconstinteger(L, n) (lua_pushstring(L, #n), lua_pushinteger(L, n), lua_settable(L, -3))

static const luaL_Reg etlib[] = {
    { "RegisterModname",          _et_RegisterModname },
    { "FindSelf",                 _et_FindSelf },
    { "FindMod",                  _et_FindMod },
    { "G_Print",                  _et_G_Print },
    { "G_Printf",                 _et_G_Printf },
    { "G_LogPrint",               _et_G_LogPrint },
    { "ConcatArgs",               _et_ConcatArgs },
    { "trap_Argc",                _et_trap_Argc },
    { "trap_Argv",                _et_trap_Argv },
    { "trap_Cvar_Get",            _et_trap_Cvar_Get },
    { "trap_Cvar_Set",            _et_trap_Cvar_Set },
    { "trap_GetConfigstring",     _et_trap_GetConfigstring },
    { "trap_SetConfigstring",     _et_trap_SetConfigstring },
    { "trap_SendConsoleCommand",  _et_trap_SendConsoleCommand },
    { "trap_SendServerCommand",   _et_trap_SendServerCommand },
    { "trap_DropClient",          _et_trap_DropClient },
    { "ClientNumberFromString",   _et_ClientNumberFromString },
    { "G_Say",                    _et_G_Say },
    { "trap_GetUserinfo",         _et_trap_GetUserinfo },
    { "trap_SetUserinfo",         _et_trap_SetUserinfo },
    { "ClientUserinfoChanged",    _et_ClientUserinfoChanged },
    { "Info_RemoveKey",           _et_Info_RemoveKey },
    { "Info_SetValueForKey",      _et_Info_SetValueForKey },
    { "Info_ValueForKey",         _et_Info_ValueForKey },
    { "Q_CleanStr",               _et_Q_CleanStr },
    { "trap_FS_FOpenFile",        _et_trap_FS_FOpenFile },
    { "trap_FS_Read",             _et_trap_FS_Read },
    { "trap_FS_Write",            _et_trap_FS_Write },
    { "trap_FS_FCloseFile",       _et_trap_FS_FCloseFile },
    { "trap_FS_Rename",           _et_trap_FS_Rename },
    { "trap_FS_GetFileList",      _et_trap_FS_GetFileList },
    { "trap_Milliseconds",        _et_trap_Milliseconds },
    { "gentity_get",              G_Lua_gentity_get },
    { "gentity_set",              G_Lua_gentity_set },
    { "G_Damage",                 _et_G_Damage },
    { "G_XP_Set",                 _et_G_XP_Set },
    { "G_ResetXP",                _et_G_ResetXP },
    { "G_AddSkillPoints",         _et_G_AddSkillPoints },
    { "G_LoseSkillPoints",        _et_G_LoseSkillPoints },
    { "G_SoundIndex",             _et_G_SoundIndex },
    { "G_ModelIndex",             _et_G_ModelIndex },
    { "G_ShaderIndex",            _et_G_ShaderIndex },
    { "G_Sound",                  _et_G_Sound },
    { "G_globalSound",            _et_G_globalSound },
    { "G_ClientSound",            _et_G_ClientSound },
    { "MutePlayer",               _et_MutePlayer },
    { "UnmutePlayer",             _et_UnmutePlayer },
    { "UnMutePlayer",             _et_UnmutePlayer },
    { "AddWeaponToPlayer",        _et_AddWeaponToPlayer },
    { "RemoveWeaponFromPlayer",   _et_RemoveWeaponFromPlayer },
    { "GetCurrentWeapon",         _et_GetCurrentWeapon },
    { "G_Spawn",                  _et_G_Spawn },
    { "G_FreeEntity",             _et_G_FreeEntity },
    { "G_EntitiesFree",           _et_G_EntitiesFree },
    { "G_TempEntity",             _et_G_TempEntity },
    { "G_AddEvent",               _et_G_AddEvent },
    { "G_SetOrigin",              _et_G_SetOrigin },
    { "G_SetAngle",               _et_G_SetAngle },
    { "G_SetEntState",            _et_G_SetEntState },
    { "trap_LinkEntity",          _et_trap_LinkEntity },
    { "trap_UnlinkEntity",        _et_trap_UnlinkEntity },
    { "trap_Trace",               _et_trap_Trace },
    { "trap_PointContents",       _et_trap_PointContents },
    { "trap_InPVS",               _et_trap_InPVS },
    { "trap_EntitiesInBox",       _et_trap_EntitiesInBox },
    { "isBitSet",                 _et_isBitSet },
    { "COM_BitCheck",             _et_COM_BitCheck },
    { "COM_BitSet",               _et_COM_BitSet },
    { "COM_BitClear",             _et_COM_BitClear },
    { "G_SHA1",                   _et_G_SHA1 },
    { "sha1",                     _et_G_SHA1 },
    { "SHA1",                     _et_G_SHA1 },
    { "IPCSend",                  G_Lua_et_IPCSend },
    { "G_GetLevelTime",           _et_G_GetLevelTime },
    { "G_GetStartTime",           _et_G_GetStartTime },
    { "PlayerName",               _et_PlayerName },
    { "IsConnected",              _et_IsConnected },
    { "trap_Cvar_GetInteger",     _et_trap_Cvar_GetInteger },
    { "Q_stricmp",                _et_Q_stricmp },
    { "FindModByName",            _et_FindModByName },
    { "G_Find",                   _et_G_Find },
    { "G_UseTargets",             _et_G_UseTargets },
    { "AddRemap",                 _et_AddRemap },
    { "trap_GetServerinfo",       _et_trap_GetServerinfo },
    { "trap_RealTime",            _et_trap_RealTime },
    { "httpRequest",              _et_httpRequest },
    { NULL, NULL }
};

void G_Lua_RegisterEtLib( lua_State *L ) {
    luaL_register( L, "et", etlib );

    lua_regconstinteger( L, EXEC_NOW );
    lua_regconstinteger( L, EXEC_INSERT );
    lua_regconstinteger( L, EXEC_APPEND );
    lua_regconstinteger( L, FS_READ );
    lua_regconstinteger( L, FS_WRITE );
    lua_regconstinteger( L, FS_APPEND );
    lua_regconstinteger( L, FS_APPEND_SYNC );
    lua_regconstinteger( L, SAY_ALL );
    lua_regconstinteger( L, SAY_TEAM );
    lua_regconstinteger( L, SAY_BUDDY );
    lua_regconstinteger( L, TEAM_FREE );
    lua_regconstinteger( L, TEAM_AXIS );
    lua_regconstinteger( L, TEAM_ALLIES );
    lua_regconstinteger( L, TEAM_SPECTATOR );
    lua_regconstinteger( L, CS_SERVERINFO );
    lua_regconstinteger( L, CS_SYSTEMINFO );
    lua_regconstinteger( L, CS_MUSIC );
    lua_regconstinteger( L, CS_MESSAGE );
    lua_regconstinteger( L, CS_MOTD );
    lua_regconstinteger( L, CS_WARMUP );
    lua_regconstinteger( L, CS_VOTE_TIME );
    lua_regconstinteger( L, CS_VOTE_STRING );
    lua_regconstinteger( L, CS_VOTE_YES );
    lua_regconstinteger( L, CS_VOTE_NO );
    lua_regconstinteger( L, CS_GAME_VERSION );
    lua_regconstinteger( L, CS_LEVEL_START_TIME );
    lua_regconstinteger( L, CS_INTERMISSION );
    lua_regconstinteger( L, CS_MULTI_INFO );
    lua_regconstinteger( L, CS_MULTI_MAPWINNER );
    lua_regconstinteger( L, CS_MULTI_OBJECTIVE );
    lua_regconstinteger( L, CS_SHADERSTATE );
    lua_regconstinteger( L, CS_PLAYERS );

    lua_regconstinteger( L, MAX_CLIENTS );
    lua_regconstinteger( L, MAX_GENTITIES );
    lua_regconstinteger( L, ENTITYNUM_NONE );
    lua_regconstinteger( L, ENTITYNUM_WORLD );
    lua_regconstinteger( L, ENTITYNUM_MAX_NORMAL );

    lua_regconstinteger( L, CON_DISCONNECTED );
    lua_regconstinteger( L, CON_CONNECTING );
    lua_regconstinteger( L, CON_CONNECTED );
    lua_regconstinteger( L, SPECTATOR_NOT );
    lua_regconstinteger( L, SPECTATOR_FREE );
    lua_regconstinteger( L, SPECTATOR_FOLLOW );

    lua_regconstinteger( L, PC_SOLDIER );
    lua_regconstinteger( L, PC_MEDIC );
    lua_regconstinteger( L, PC_ENGINEER );
    lua_regconstinteger( L, PC_FIELDOPS );
    lua_regconstinteger( L, PC_COVERTOPS );

    lua_regconstinteger( L, SK_BATTLE_SENSE );
    lua_regconstinteger( L, SK_EXPLOSIVES_AND_CONSTRUCTION );
    lua_regconstinteger( L, SK_FIRST_AID );
    lua_regconstinteger( L, SK_SIGNALS );
    lua_regconstinteger( L, SK_LIGHT_WEAPONS );
    lua_regconstinteger( L, SK_HEAVY_WEAPONS );
    lua_regconstinteger( L, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS );
    lua_regconstinteger( L, SK_NUM_SKILLS );

    lua_regconstinteger( L, STAT_HEALTH );
    lua_regconstinteger( L, STAT_KEYS );
    lua_regconstinteger( L, STAT_DEAD_YAW );
    lua_regconstinteger( L, STAT_MAX_HEALTH );
    lua_regconstinteger( L, STAT_PLAYER_CLASS );
    lua_regconstinteger( L, STAT_XP );

    lua_regconstinteger( L, PM_NORMAL );
    lua_regconstinteger( L, PM_NOCLIP );
    lua_regconstinteger( L, PM_SPECTATOR );
    lua_regconstinteger( L, PM_DEAD );
    lua_regconstinteger( L, PM_FREEZE );
    lua_regconstinteger( L, PM_INTERMISSION );

    lua_regconstinteger( L, STATE_DEFAULT );
    lua_regconstinteger( L, STATE_INVISIBLE );
    lua_regconstinteger( L, STATE_UNDERCONSTRUCTION );

    lua_regconstinteger( L, SVF_NOCLIENT );
    lua_regconstinteger( L, SVF_BROADCAST );
    lua_regconstinteger( L, SVF_BOT );

    lua_regconstinteger( L, DAMAGE_RADIUS );
    lua_regconstinteger( L, DAMAGE_NO_KNOCKBACK );
    lua_regconstinteger( L, DAMAGE_NO_PROTECTION );
    lua_regconstinteger( L, DAMAGE_NO_TEAM_PROTECTION );
    lua_regconstinteger( L, DAMAGE_DISTANCEFALLOFF );

    lua_regconstinteger( L, CONTENTS_SOLID );
    lua_regconstinteger( L, CONTENTS_BODY );
    lua_regconstinteger( L, CONTENTS_CORPSE );
    lua_regconstinteger( L, CONTENTS_PLAYERCLIP );
    lua_regconstinteger( L, CONTENTS_TRIGGER );
    lua_regconstinteger( L, MASK_SOLID );
    lua_regconstinteger( L, MASK_PLAYERSOLID );
    lua_regconstinteger( L, MASK_SHOT );
    lua_regconstinteger( L, MASK_MISSILESHOT );

    lua_regconstinteger( L, WP_NONE );
    lua_regconstinteger( L, WP_KNIFE );
    lua_regconstinteger( L, WP_LUGER );
    lua_regconstinteger( L, WP_MP40 );
    lua_regconstinteger( L, WP_GRENADE_LAUNCHER );
    lua_regconstinteger( L, WP_PANZERFAUST );
    lua_regconstinteger( L, WP_FLAMETHROWER );
    lua_regconstinteger( L, WP_COLT );
    lua_regconstinteger( L, WP_THOMPSON );
    lua_regconstinteger( L, WP_GRENADE_PINEAPPLE );
    lua_regconstinteger( L, WP_STEN );
    lua_regconstinteger( L, WP_MEDIC_SYRINGE );
    lua_regconstinteger( L, WP_AMMO );
    lua_regconstinteger( L, WP_ARTY );
    lua_regconstinteger( L, WP_SILENCER );
    lua_regconstinteger( L, WP_DYNAMITE );
    lua_regconstinteger( L, WP_MEDKIT );
    lua_regconstinteger( L, WP_BINOCULARS );
    lua_regconstinteger( L, WP_PLIERS );
    lua_regconstinteger( L, WP_SMOKE_MARKER );
    lua_regconstinteger( L, WP_KAR98 );
    lua_regconstinteger( L, WP_CARBINE );
    lua_regconstinteger( L, WP_GARAND );
    lua_regconstinteger( L, WP_LANDMINE );
    lua_regconstinteger( L, WP_SATCHEL );
    lua_regconstinteger( L, WP_SATCHEL_DET );
    lua_regconstinteger( L, WP_SMOKE_BOMB );
    lua_regconstinteger( L, WP_MOBILE_MG42 );
    lua_regconstinteger( L, WP_K43 );
    lua_regconstinteger( L, WP_FG42 );
    lua_regconstinteger( L, WP_MORTAR );
    lua_regconstinteger( L, WP_AKIMBO_COLT );
    lua_regconstinteger( L, WP_AKIMBO_LUGER );
    lua_regconstinteger( L, WP_GPG40 );
    lua_regconstinteger( L, WP_M7 );
    lua_regconstinteger( L, WP_SILENCED_COLT );
    lua_regconstinteger( L, WP_GARAND_SCOPE );
    lua_regconstinteger( L, WP_K43_SCOPE );
    lua_regconstinteger( L, WP_FG42SCOPE );
    lua_regconstinteger( L, WP_MORTAR_SET );
    lua_regconstinteger( L, WP_MEDIC_ADRENALINE );
    lua_regconstinteger( L, WP_AKIMBO_SILENCEDCOLT );
    lua_regconstinteger( L, WP_AKIMBO_SILENCEDLUGER );
    lua_regconstinteger( L, WP_MOBILE_MG42_SET );
    lua_regconstinteger( L, WP_POISON_SYRINGE );
    lua_regconstinteger( L, WP_M97 );
    lua_regconstinteger( L, WP_POISON_GAS );
    lua_regconstinteger( L, WP_MOLOTOV );
    lua_regconstinteger( L, WP_NUM_WEAPONS );

    lua_regconstinteger( L, MOD_UNKNOWN );
    lua_regconstinteger( L, MOD_MACHINEGUN );
    lua_regconstinteger( L, MOD_BROWNING );
    lua_regconstinteger( L, MOD_MG42 );
    lua_regconstinteger( L, MOD_GRENADE );
    lua_regconstinteger( L, MOD_ROCKET );
    lua_regconstinteger( L, MOD_KNIFE );
    lua_regconstinteger( L, MOD_LUGER );
    lua_regconstinteger( L, MOD_COLT );
    lua_regconstinteger( L, MOD_MP40 );
    lua_regconstinteger( L, MOD_THOMPSON );
    lua_regconstinteger( L, MOD_STEN );
    lua_regconstinteger( L, MOD_GARAND );
    lua_regconstinteger( L, MOD_SILENCER );
    lua_regconstinteger( L, MOD_FG42 );
    lua_regconstinteger( L, MOD_FG42SCOPE );
    lua_regconstinteger( L, MOD_PANZERFAUST );
    lua_regconstinteger( L, MOD_GRENADE_LAUNCHER );
    lua_regconstinteger( L, MOD_FLAMETHROWER );
    lua_regconstinteger( L, MOD_GRENADE_PINEAPPLE );
    lua_regconstinteger( L, MOD_DYNAMITE );
    lua_regconstinteger( L, MOD_AIRSTRIKE );
    lua_regconstinteger( L, MOD_SYRINGE );
    lua_regconstinteger( L, MOD_ARTY );
    lua_regconstinteger( L, MOD_WATER );
    lua_regconstinteger( L, MOD_CRUSH );
    lua_regconstinteger( L, MOD_TELEFRAG );
    lua_regconstinteger( L, MOD_FALLING );
    lua_regconstinteger( L, MOD_SUICIDE );
    lua_regconstinteger( L, MOD_TRIGGER_HURT );
    lua_regconstinteger( L, MOD_EXPLOSIVE );
    lua_regconstinteger( L, MOD_CARBINE );
    lua_regconstinteger( L, MOD_KAR98 );
    lua_regconstinteger( L, MOD_GPG40 );
    lua_regconstinteger( L, MOD_M7 );
    lua_regconstinteger( L, MOD_LANDMINE );
    lua_regconstinteger( L, MOD_SATCHEL );
    lua_regconstinteger( L, MOD_SMOKEBOMB );
    lua_regconstinteger( L, MOD_MOBILE_MG42 );
    lua_regconstinteger( L, MOD_GARAND_SCOPE );
    lua_regconstinteger( L, MOD_K43 );
    lua_regconstinteger( L, MOD_K43_SCOPE );
    lua_regconstinteger( L, MOD_MORTAR );
    lua_regconstinteger( L, MOD_AKIMBO_COLT );
    lua_regconstinteger( L, MOD_AKIMBO_LUGER );
    lua_regconstinteger( L, MOD_SWITCHTEAM );
    lua_regconstinteger( L, MOD_GOOMBA );
    lua_regconstinteger( L, MOD_POISON_SYRINGE );
    lua_regconstinteger( L, MOD_THROWING_KNIFE );
    lua_regconstinteger( L, MOD_M97 );
    lua_regconstinteger( L, MOD_POISON_GAS );
    lua_regconstinteger( L, MOD_MOLOTOV );
    lua_regconstinteger( L, MOD_NUM_MODS );

    lua_pop( L, 1 );
}

void G_Lua_RegisterFields( lua_State *L ) {
    (void)L;
}

#endif
