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

    session = WinHttpOpen( L"Jaymod-Lua/3.0.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
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
    lua_regconstinteger( L, CS_PLAYERS );
    lua_regconstinteger( L, CS_SERVERINFO );
    lua_regconstinteger( L, CS_SYSTEMINFO );
    lua_regconstinteger( L, MOD_UNKNOWN );
    lua_regconstinteger( L, MOD_MACHINEGUN );
    lua_regconstinteger( L, MOD_SUICIDE );
    lua_regconstinteger( L, MAX_CLIENTS );

    lua_pop( L, 1 );
}

void G_Lua_RegisterFields( lua_State *L ) {
    (void)L;
}

#endif
