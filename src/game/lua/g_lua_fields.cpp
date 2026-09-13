#include <bgame/impl.h>
#include <game/lua/g_lua.h>
#include <stddef.h>

#ifdef FEATURE_LUA

#define FIELD_INT         0
#define FIELD_STRING      1
#define FIELD_FLOAT       2
#define FIELD_ENTITY      3
#define FIELD_VEC3        4
#define FIELD_INT_ARRAY   5
#define FIELD_FLOAT_ARRAY 7
#define FIELD_STRINGBUF   9

#define FIELD_FLAG_GENTITY  1
#define FIELD_FLAG_GCLIENT  2
#define FIELD_FLAG_READONLY 8

#define _et_gentity_addfield(n, t, f) { #n, t, offsetof(struct gentity_s, n), FIELD_FLAG_GENTITY + (f) }
#define _et_gclient_addfield(n, t, f) { #n, t, offsetof(struct gclient_s, n), FIELD_FLAG_GCLIENT + (f) }
#define _et_gclient_addfieldalias(n, a, t, f) { #n, t, offsetof(struct gclient_s, a), FIELD_FLAG_GCLIENT + (f) }

typedef struct {
    const char *name;
    int type;
    size_t mapping;
    int flags;
} gentity_field_t;

static const gentity_field_t fields[] = {
    _et_gentity_addfield( classname, FIELD_STRING, FIELD_FLAG_READONLY ),
    _et_gentity_addfield( inuse, FIELD_INT, FIELD_FLAG_READONLY ),
    _et_gentity_addfield( spawnflags, FIELD_INT, 0 ),
    _et_gentity_addfield( flags, FIELD_INT, 0 ),
    _et_gentity_addfield( health, FIELD_INT, 0 ),
    _et_gentity_addfield( damage, FIELD_INT, 0 ),
    _et_gentity_addfield( splashDamage, FIELD_INT, 0 ),
    _et_gentity_addfield( splashRadius, FIELD_INT, 0 ),
    _et_gentity_addfield( count, FIELD_INT, 0 ),
    _et_gentity_addfield( count2, FIELD_INT, 0 ),
    _et_gentity_addfield( timestamp, FIELD_INT, 0 ),
    _et_gentity_addfield( nextthink, FIELD_INT, 0 ),
    _et_gentity_addfield( wait, FIELD_FLOAT, 0 ),
    _et_gentity_addfield( random, FIELD_FLOAT, 0 ),
    _et_gentity_addfield( delay, FIELD_FLOAT, 0 ),
    _et_gentity_addfield( radius, FIELD_INT, 0 ),
    _et_gentity_addfield( target, FIELD_STRING, FIELD_FLAG_READONLY ),
    _et_gentity_addfield( targetname, FIELD_STRING, FIELD_FLAG_READONLY ),
    _et_gentity_addfield( message, FIELD_STRING, FIELD_FLAG_READONLY ),
    _et_gentity_addfield( model, FIELD_STRING, FIELD_FLAG_READONLY ),
    _et_gentity_addfield( clipmask, FIELD_INT, 0 ),
    { "s.number", FIELD_INT, offsetof(struct gentity_s, s.number), FIELD_FLAG_GENTITY | FIELD_FLAG_READONLY },
    { "s.eType", FIELD_INT, offsetof(struct gentity_s, s.eType), FIELD_FLAG_GENTITY },
    { "s.eFlags", FIELD_INT, offsetof(struct gentity_s, s.eFlags), FIELD_FLAG_GENTITY },
    { "s.weapon", FIELD_INT, offsetof(struct gentity_s, s.weapon), FIELD_FLAG_GENTITY },
    { "s.teamNum", FIELD_INT, offsetof(struct gentity_s, s.teamNum), FIELD_FLAG_GENTITY },
    { "s.origin", FIELD_VEC3, offsetof(struct gentity_s, s.origin), FIELD_FLAG_GENTITY },
    { "s.angles", FIELD_VEC3, offsetof(struct gentity_s, s.angles), FIELD_FLAG_GENTITY },
    { "s.origin2", FIELD_VEC3, offsetof(struct gentity_s, s.origin2), FIELD_FLAG_GENTITY },
    { "r.currentOrigin", FIELD_VEC3, offsetof(struct gentity_s, r.currentOrigin), FIELD_FLAG_GENTITY },
    { "r.currentAngles", FIELD_VEC3, offsetof(struct gentity_s, r.currentAngles), FIELD_FLAG_GENTITY },
    { "r.mins", FIELD_VEC3, offsetof(struct gentity_s, r.mins), FIELD_FLAG_GENTITY },
    { "r.maxs", FIELD_VEC3, offsetof(struct gentity_s, r.maxs), FIELD_FLAG_GENTITY },
    { "r.contents", FIELD_INT, offsetof(struct gentity_s, r.contents), FIELD_FLAG_GENTITY },
    { "r.ownerNum", FIELD_INT, offsetof(struct gentity_s, r.ownerNum), FIELD_FLAG_GENTITY },
    { "r.svFlags", FIELD_INT, offsetof(struct gentity_s, r.svFlags), FIELD_FLAG_GENTITY },
    { "r.linked", FIELD_INT, offsetof(struct gentity_s, r.linked), FIELD_FLAG_GENTITY | FIELD_FLAG_READONLY },
    { "s.pos.trBase", FIELD_VEC3, offsetof(struct gentity_s, s.pos.trBase), FIELD_FLAG_GENTITY },
    { "s.pos.trDelta", FIELD_VEC3, offsetof(struct gentity_s, s.pos.trDelta), FIELD_FLAG_GENTITY },
    { "s.apos.trBase", FIELD_VEC3, offsetof(struct gentity_s, s.apos.trBase), FIELD_FLAG_GENTITY },
    { "s.event", FIELD_INT, offsetof(struct gentity_s, s.event), FIELD_FLAG_GENTITY },
    { "s.eventParm", FIELD_INT, offsetof(struct gentity_s, s.eventParm), FIELD_FLAG_GENTITY },
    { "s.otherEntityNum", FIELD_INT, offsetof(struct gentity_s, s.otherEntityNum), FIELD_FLAG_GENTITY },
    { "s.otherEntityNum2", FIELD_INT, offsetof(struct gentity_s, s.otherEntityNum2), FIELD_FLAG_GENTITY },
    { "s.groundEntityNum", FIELD_INT, offsetof(struct gentity_s, s.groundEntityNum), FIELD_FLAG_GENTITY },
    _et_gentity_addfield( parent, FIELD_ENTITY, 0 ),
    _et_gentity_addfield( enemy, FIELD_ENTITY, 0 ),
    _et_gentity_addfield( activator, FIELD_ENTITY, 0 ),
    _et_gentity_addfield( takedamage, FIELD_INT, 0 ),
    _et_gentity_addfield( methodOfDeath, FIELD_INT, 0 ),
    _et_gentity_addfield( splashMethodOfDeath, FIELD_INT, 0 ),
    _et_gentity_addfield( waterlevel, FIELD_INT, 0 ),

    _et_gclient_addfieldalias( sess.sessionTeam, sess.sessionTeam, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.spectatorState, sess.spectatorState, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.spectatorClient, sess.spectatorClient, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.playerType, sess.playerType, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.playerWeapon, sess.playerWeapon, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.playerWeapon2, sess.playerWeapon2, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.latchPlayerType, sess.latchPlayerType, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.kills, sess.kills, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.deaths, sess.deaths, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.suicides, sess.suicides, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.team_kills, sess.team_kills, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.team_damage, sess.team_damage, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.damage_given, sess.damage_given, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.damage_received, sess.damage_received, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.referee, sess.referee, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.rank, sess.rank, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.revives, sess.revives, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.headshots, sess.headshots, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.shoutcaster, sess.shoutcaster, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( sess.skillpoints, sess.skillpoints, FIELD_FLOAT_ARRAY, 0 ),
    _et_gclient_addfieldalias( sess.skill, sess.skill, FIELD_INT_ARRAY, 0 ),
    _et_gclient_addfieldalias( pers.netname, pers.netname, FIELD_STRINGBUF, FIELD_FLAG_READONLY ),
    _et_gclient_addfieldalias( pers.connected, pers.connected, FIELD_INT, FIELD_FLAG_READONLY ),
    _et_gclient_addfieldalias( pers.enterTime, pers.enterTime, FIELD_INT, FIELD_FLAG_READONLY ),
    _et_gclient_addfieldalias( pers.playerName, pers.netname, FIELD_STRINGBUF, FIELD_FLAG_READONLY ),
    _et_gclient_addfieldalias( ps.origin, ps.origin, FIELD_VEC3, 0 ),
    _et_gclient_addfieldalias( ps.velocity, ps.velocity, FIELD_VEC3, 0 ),
    _et_gclient_addfieldalias( ps.viewangles, ps.viewangles, FIELD_VEC3, 0 ),
    _et_gclient_addfieldalias( ps.stats, ps.stats, FIELD_INT_ARRAY, 0 ),
    _et_gclient_addfieldalias( ps.persistant, ps.persistant, FIELD_INT_ARRAY, 0 ),
    _et_gclient_addfieldalias( ps.powerups, ps.powerups, FIELD_INT_ARRAY, 0 ),
    _et_gclient_addfieldalias( ps.ammo, ps.ammo, FIELD_INT_ARRAY, 0 ),
    _et_gclient_addfieldalias( ps.ammoclip, ps.ammoclip, FIELD_INT_ARRAY, 0 ),
    _et_gclient_addfieldalias( ps.weapon, ps.weapon, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( ps.ping, ps.ping, FIELD_INT, FIELD_FLAG_READONLY ),
    _et_gclient_addfieldalias( ps.pm_type, ps.pm_type, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( ps.pm_flags, ps.pm_flags, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( ps.clientNum, ps.clientNum, FIELD_INT, FIELD_FLAG_READONLY ),
    _et_gclient_addfieldalias( ps.viewheight, ps.viewheight, FIELD_INT, 0 ),
    _et_gclient_addfieldalias( noclip, noclip, FIELD_INT, 0 ),

    { NULL, 0, 0, 0 }
};

static const gentity_field_t *G_Lua_FindField( const char *name ) {
    int i;
    for ( i = 0; fields[i].name; i++ ) {
        if ( !Q_stricmp( name, fields[i].name ) ) {
            return &fields[i];
        }
    }
    return NULL;
}

static unsigned char *G_Lua_FieldPtr( int entnum, const gentity_field_t *f ) {
    gentity_t *ent;
    if ( entnum < 0 || entnum >= MAX_GENTITIES ) {
        return NULL;
    }
    ent = &g_entities[entnum];
    if ( f->flags & FIELD_FLAG_GCLIENT ) {
        if ( !ent->client ) {
            return NULL;
        }
        return (unsigned char *)ent->client + f->mapping;
    }
    return (unsigned char *)ent + f->mapping;
}

int G_Lua_gentity_get( lua_State *L ) {
    int entnum = luaL_checkint( L, 1 );
    const char *name = luaL_checkstring( L, 2 );
    const gentity_field_t *f = G_Lua_FindField( name );
    unsigned char *ptr;
    int index = 0;

    if ( !f ) {
        lua_pushnil( L );
        return 1;
    }
    ptr = G_Lua_FieldPtr( entnum, f );
    if ( !ptr ) {
        lua_pushnil( L );
        return 1;
    }

    if ( lua_gettop( L ) >= 3 ) {
        index = luaL_checkint( L, 3 );
    }

    switch ( f->type ) {
    case FIELD_INT:
        lua_pushinteger( L, *(int *)ptr );
        break;
    case FIELD_FLOAT:
        lua_pushnumber( L, *(float *)ptr );
        break;
    case FIELD_STRING:
        lua_pushstring( L, *(char **)ptr ? *(char **)ptr : "" );
        break;
    case FIELD_STRINGBUF:
        lua_pushstring( L, (char *)ptr );
        break;
    case FIELD_ENTITY: {
        gentity_t *e = *(gentity_t **)ptr;
        if ( e ) {
            lua_pushinteger( L, e - g_entities );
        } else {
            lua_pushnil( L );
        }
        break;
    }
    case FIELD_VEC3: {
        float *v = (float *)ptr;
        lua_newtable( L );
        lua_pushnumber( L, v[0] ); lua_rawseti( L, -2, 1 );
        lua_pushnumber( L, v[1] ); lua_rawseti( L, -2, 2 );
        lua_pushnumber( L, v[2] ); lua_rawseti( L, -2, 3 );
        break;
    }
    case FIELD_INT_ARRAY:
        lua_pushinteger( L, ( (int *)ptr )[index] );
        break;
    case FIELD_FLOAT_ARRAY:
        lua_pushnumber( L, ( (float *)ptr )[index] );
        break;
    default:
        lua_pushnil( L );
        break;
    }
    return 1;
}

int G_Lua_gentity_set( lua_State *L ) {
    int entnum = luaL_checkint( L, 1 );
    const char *name = luaL_checkstring( L, 2 );
    const gentity_field_t *f = G_Lua_FindField( name );
    unsigned char *ptr;
    int valueIndex = 3;
    int index = 0;

    if ( !f || ( f->flags & FIELD_FLAG_READONLY ) ) {
        return 0;
    }
    ptr = G_Lua_FieldPtr( entnum, f );
    if ( !ptr ) {
        return 0;
    }

    if ( f->type == FIELD_INT_ARRAY || f->type == FIELD_FLOAT_ARRAY ) {
        index = luaL_checkint( L, 3 );
        valueIndex = 4;
    }

    switch ( f->type ) {
    case FIELD_INT:
        *(int *)ptr = luaL_checkint( L, valueIndex );
        break;
    case FIELD_FLOAT:
        *(float *)ptr = (float)luaL_checknumber( L, valueIndex );
        break;
    case FIELD_VEC3:
        if ( lua_istable( L, valueIndex ) ) {
            float *v = (float *)ptr;
            lua_rawgeti( L, valueIndex, 1 ); v[0] = (float)lua_tonumber( L, -1 ); lua_pop( L, 1 );
            lua_rawgeti( L, valueIndex, 2 ); v[1] = (float)lua_tonumber( L, -1 ); lua_pop( L, 1 );
            lua_rawgeti( L, valueIndex, 3 ); v[2] = (float)lua_tonumber( L, -1 ); lua_pop( L, 1 );
        }
        break;
    case FIELD_INT_ARRAY:
        ( (int *)ptr )[index] = luaL_checkint( L, valueIndex );
        break;
    case FIELD_FLOAT_ARRAY:
        ( (float *)ptr )[index] = (float)luaL_checknumber( L, valueIndex );
        break;
    case FIELD_ENTITY:
        if ( lua_isnil( L, valueIndex ) ) {
            *(gentity_t **)ptr = NULL;
        } else {
            int n = luaL_checkint( L, valueIndex );
            *(gentity_t **)ptr = ( n >= 0 && n < MAX_GENTITIES ) ? &g_entities[n] : NULL;
        }
        break;
    default:
        break;
    }
    return 0;
}

#endif
