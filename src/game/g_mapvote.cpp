#include <bgame/impl.h>
#include <game/g_mapvote.h>

#define MAX_MAPVOTE_POOL 512
#define MAPVOTEINFO_FILE "mapvoteinfo.txt"
#define MAPVOTE_LIST_BUF 98304

typedef struct {
	char bsp[MAX_QPATH];
	char longname[64];
	int age;
	int plays;
} mapVotePool_t;

static mapVotePool_t pool[MAX_MAPVOTE_POOL];
static int poolCount;
static int ballot[MAX_MAPVOTE_MAPS];
static int ballotCount;
static int ballotVotes[MAX_MAPVOTE_MAPS];

static int G_MapVote_FindPool( const char *bsp ) {
	int i;
	for ( i = 0; i < poolCount; i++ ) {
		if ( !Q_stricmp( pool[i].bsp, bsp ) ) {
			return i;
		}
	}
	return -1;
}

static qboolean G_MapVote_Excluded( const char *bsp ) {
	char wrap[MAX_STRING_CHARS];
	char hay[MAX_STRING_CHARS];
	char needle[MAX_QPATH + 4];

	if ( !g_excludedMaps.string[0] ) {
		return qfalse;
	}
	Q_strncpyz( hay, g_excludedMaps.string, sizeof( hay ) );
	Q_strlwr( hay );
	Com_sprintf( wrap, sizeof( wrap ), ":%s:", hay );
	Com_sprintf( needle, sizeof( needle ), ":%s:", bsp );
	Q_strlwr( needle );
	return strstr( wrap, needle ) ? qtrue : qfalse;
}

static void G_MapVote_ParseArenaFile( const char *filename ) {
	fileHandle_t f;
	int len;
	char buf[4096];
	char *p;
	char token[MAX_TOKEN_CHARS];
	char mapname[MAX_QPATH];
	char longname[64];
	qboolean haveMap;

	len = trap_FS_FOpenFile( va( "scripts/%s", filename ), &f, FS_READ );
	if ( len <= 0 ) {
		if ( f ) {
			trap_FS_FCloseFile( f );
		}
		return;
	}
	if ( len >= (int)sizeof( buf ) - 1 ) {
		len = (int)sizeof( buf ) - 1;
	}
	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	p = buf;
	haveMap = qfalse;
	mapname[0] = 0;
	longname[0] = 0;
	while ( 1 ) {
		char *tok = COM_Parse( &p );
		if ( !tok[0] ) {
			break;
		}
		Q_strncpyz( token, tok, sizeof( token ) );
		if ( !Q_stricmp( token, "map" ) ) {
			tok = COM_Parse( &p );
			if ( tok[0] ) {
				if ( haveMap && mapname[0] ) {
					int idx = G_MapVote_FindPool( mapname );
					if ( idx >= 0 && longname[0] ) {
						Q_strncpyz( pool[idx].longname, longname, sizeof( pool[idx].longname ) );
					}
				}
				Q_strncpyz( mapname, tok, sizeof( mapname ) );
				longname[0] = 0;
				haveMap = qtrue;
			}
		} else if ( !Q_stricmp( token, "longname" ) ) {
			tok = COM_Parse( &p );
			if ( tok[0] ) {
				Q_strncpyz( longname, tok, sizeof( longname ) );
			}
		}
	}
	if ( haveMap && mapname[0] ) {
		int idx = G_MapVote_FindPool( mapname );
		if ( idx >= 0 && longname[0] ) {
			Q_strncpyz( pool[idx].longname, longname, sizeof( pool[idx].longname ) );
		}
	}
}

static void G_MapVote_LoadArenas( void ) {
	static char list[MAPVOTE_LIST_BUF];
	char *p;
	int i, n;

	n = trap_FS_GetFileList( "scripts", ".arena", list, sizeof( list ) );
	p = list;
	for ( i = 0; i < n; i++ ) {
		G_MapVote_ParseArenaFile( p );
		p += strlen( p ) + 1;
	}
}

static void G_MapVote_ReadInfo( void ) {
	fileHandle_t f;
	int len, i;
	char buf[16384];
	char *p;

	len = trap_FS_FOpenFile( MAPVOTEINFO_FILE, &f, FS_READ );
	if ( len <= 0 ) {
		if ( f ) {
			trap_FS_FCloseFile( f );
		}
		return;
	}
	if ( len >= (int)sizeof( buf ) - 1 ) {
		len = (int)sizeof( buf ) - 1;
	}
	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	p = buf;
	while ( 1 ) {
		char name[MAX_QPATH];
		char *tok;
		int age = 0, plays = 0;
		tok = COM_Parse( &p );
		if ( !tok[0] ) {
			break;
		}
		Q_strncpyz( name, tok, sizeof( name ) );
		tok = COM_Parse( &p );
		if ( tok[0] ) {
			age = atoi( tok );
		}
		tok = COM_Parse( &p );
		if ( tok[0] ) {
			plays = atoi( tok );
		}
		i = G_MapVote_FindPool( name );
		if ( i >= 0 ) {
			pool[i].age = age;
			pool[i].plays = plays;
		}
	}
}

static void G_MapVote_WriteInfo( void ) {
	fileHandle_t f;
	int i;
	char line[256];

	trap_FS_FOpenFile( MAPVOTEINFO_FILE, &f, FS_WRITE );
	if ( !f ) {
		return;
	}
	for ( i = 0; i < poolCount; i++ ) {
		Com_sprintf( line, sizeof( line ), "%s %i %i\n", pool[i].bsp, pool[i].age, pool[i].plays );
		trap_FS_Write( line, (int)strlen( line ), f );
	}
	trap_FS_FCloseFile( f );
}

static qboolean G_MapVote_InfoFits( const char *cs, const char *key, const char *val ) {
	char test[MAX_INFO_STRING];
	Q_strncpyz( test, cs, sizeof( test ) );
	Info_SetValueForKey( test, key, val );
	return Info_ValueForKey( test, key )[0] ? qtrue : qfalse;
}

static void G_MapVote_SanitizeInfoVal( char *s ) {
	int i;
	for ( i = 0; s[i]; i++ ) {
		if ( s[i] == '\\' || s[i] == ';' || s[i] == '"' ) {
			s[i] = ' ';
		}
	}
}

static void G_MapVote_WriteInfoCS( const int *csIds, int ncs, qboolean longs ) {
	char buf[MAX_INFO_STRING];
	int i, slot, local;

	buf[0] = 0;
	slot = 0;
	local = 0;
	for ( i = 0; i < ncs; i++ ) {
		trap_SetConfigstring( csIds[i], "" );
	}

	for ( i = 0; i < ballotCount && slot < ncs; i++ ) {
		char val[64];
		int p = ballot[i];
		Q_strncpyz( val, longs ? ( pool[p].longname[0] ? pool[p].longname : pool[p].bsp ) : pool[p].bsp, sizeof( val ) );
		G_MapVote_SanitizeInfoVal( val );
		if ( !G_MapVote_InfoFits( buf, va( "%i", local ), val ) ) {
			trap_SetConfigstring( csIds[slot], buf );
			slot++;
			local = 0;
			buf[0] = 0;
			if ( slot >= ncs ) {
				break;
			}
		}
		Info_SetValueForKey( buf, va( "%i", local ), val );
		local++;
	}
	if ( slot < ncs ) {
		trap_SetConfigstring( csIds[slot], buf );
	}
}

static void G_MapVote_UpdateConfigString( void ) {
	char cs[MAX_INFO_STRING];
	int i;
	const int mapCs[2] = { CS_MAPVOTE_MAPS, CS_MAPVOTE_MAPS2 };
	const int longCs[3] = { CS_MAPVOTE_LONG, CS_MAPVOTE_LONG2, CS_MAPVOTE_LONG3 };

	cs[0] = 0;
	Info_SetValueForKey( cs, "n", va( "%i", ballotCount ) );
	Info_SetValueForKey( cs, "f", va( "%i", g_mapVoteFlags.integer | MAPVOTE_MULTI_VOTE ) );
	for ( i = 0; i < ballotCount; i++ ) {
		Info_SetValueForKey( cs, va( "v%i", i ), va( "%i", ballotVotes[i] ) );
	}
	trap_SetConfigstring( CS_MAPVOTE, cs );
	G_MapVote_WriteInfoCS( mapCs, 2, qfalse );
	G_MapVote_WriteInfoCS( longCs, 3, qtrue );
}

static int G_MapVote_RankWeight( int rank ) {
	if ( rank == 0 ) {
		return 3;
	}
	if ( rank == 1 ) {
		return 2;
	}
	return 1;
}

static void G_MapVote_Recount( void ) {
	int i, r, idx;
	gclient_t *cl;

	memset( ballotVotes, 0, sizeof( ballotVotes ) );
	for ( i = 0; i < level.numConnectedClients; i++ ) {
		cl = level.clients + level.sortedClients[i];
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		for ( r = 0; r < 3; r++ ) {
			idx = cl->pers.mapVote[r] - 1;
			if ( idx < 0 || idx >= ballotCount ) {
				continue;
			}
			ballotVotes[idx] += G_MapVote_RankWeight( r );
		}
	}
	G_MapVote_UpdateConfigString();
}

static void G_MapVote_Shuffle( int *idx, int n ) {
	int i, j, t;
	for ( i = n - 1; i > 0; i-- ) {
		j = rand() % ( i + 1 );
		t = idx[i];
		idx[i] = idx[j];
		idx[j] = t;
	}
}

void G_MapVote_Init( void ) {
	static char list[MAPVOTE_LIST_BUF];
	char *p;
	char *end;
	int i, n, cur, skipped, parsed;

	poolCount = 0;
	ballotCount = 0;
	memset( ballotVotes, 0, sizeof( ballotVotes ) );
	trap_SetConfigstring( CS_MAPVOTE, "" );
	trap_SetConfigstring( CS_MAPVOTE_MAPS, "" );
	trap_SetConfigstring( CS_MAPVOTE_MAPS2, "" );
	trap_SetConfigstring( CS_MAPVOTE_LONG, "" );
	trap_SetConfigstring( CS_MAPVOTE_LONG2, "" );
	trap_SetConfigstring( CS_MAPVOTE_LONG3, "" );

	if ( g_gametype.integer != GT_WOLF_MAPVOTE ) {
		return;
	}

	n = trap_FS_GetFileList( "maps", ".bsp", list, sizeof( list ) );
	p = list;
	end = list + sizeof( list );
	skipped = 0;
	parsed = 0;
	for ( i = 0; i < n && poolCount < MAX_MAPVOTE_POOL && p < end && p[0]; i++ ) {
		char name[MAX_QPATH];
		int namelen;

		Q_strncpyz( name, p, sizeof( name ) );
		p += strlen( p ) + 1;
		parsed++;
		namelen = (int)strlen( name );
		if ( namelen > 4 && !Q_stricmp( name + namelen - 4, ".bsp" ) ) {
			name[namelen - 4] = 0;
		}
		if ( !name[0] ) {
			continue;
		}
		if ( G_MapVote_FindPool( name ) >= 0 ) {
			continue;
		}
		if ( G_MapVote_Excluded( name ) ) {
			G_Printf( "Map vote: excluded %s\n", name );
			skipped++;
			continue;
		}
		Q_strncpyz( pool[poolCount].bsp, name, sizeof( pool[poolCount].bsp ) );
		Q_strncpyz( pool[poolCount].longname, name, sizeof( pool[poolCount].longname ) );
		pool[poolCount].age = 0;
		pool[poolCount].plays = 0;
		poolCount++;
	}

	if ( parsed < n ) {
		G_Printf( "Map vote: file list truncated (%i names read, FS reported %i)\n", parsed, n );
	}

	G_MapVote_ReadInfo();
	G_MapVote_LoadArenas();

	for ( i = 0; i < poolCount; i++ ) {
		pool[i].age++;
	}
	cur = G_MapVote_FindPool( level.rawmapname );
	if ( cur >= 0 ) {
		pool[cur].age = 0;
		pool[cur].plays++;
	}
	G_MapVote_WriteInfo();
	G_Printf( "Map vote: %i maps in pool (%i excluded, %i .bsp from FS)\n", poolCount, skipped, n );
}

void G_MapVote_BeginIntermission( void ) {
	int eligible[MAX_MAPVOTE_POOL];
	int eligCount = 0;
	int want, i, minAge;
	qboolean showAll;

	ballotCount = 0;
	memset( ballotVotes, 0, sizeof( ballotVotes ) );

	if ( g_gametype.integer != GT_WOLF_MAPVOTE || poolCount <= 0 ) {
		return;
	}

	showAll = ( g_maxMapsVotedFor.integer <= 0 ) ? qtrue : qfalse;
	want = g_maxMapsVotedFor.integer;
	if ( want <= 0 || want > MAX_MAPVOTE_MAPS ) {
		want = MAX_MAPVOTE_MAPS;
	}
	minAge = g_minMapAge.integer;
	if ( minAge < 0 ) {
		minAge = 0;
	}

	for ( i = 0; i < poolCount; i++ ) {
		if ( G_MapVote_Excluded( pool[i].bsp ) ) {
			continue;
		}
		if ( !showAll && !Q_stricmp( pool[i].bsp, level.rawmapname ) ) {
			continue;
		}
		if ( !showAll && pool[i].age < minAge ) {
			continue;
		}
		eligible[eligCount++] = i;
	}

	if ( !showAll && eligCount < want ) {
		eligCount = 0;
		for ( i = 0; i < poolCount; i++ ) {
			if ( G_MapVote_Excluded( pool[i].bsp ) ) {
				continue;
			}
			if ( !Q_stricmp( pool[i].bsp, level.rawmapname ) ) {
				continue;
			}
			eligible[eligCount++] = i;
		}
	}

	if ( eligCount <= 0 ) {
		for ( i = 0; i < poolCount && eligCount < MAX_MAPVOTE_POOL; i++ ) {
			if ( G_MapVote_Excluded( pool[i].bsp ) ) {
				continue;
			}
			eligible[eligCount++] = i;
		}
	}

	if ( !( g_mapVoteFlags.integer & MAPVOTE_NO_RANDOMIZE ) ) {
		G_MapVote_Shuffle( eligible, eligCount );
	}

	if ( eligCount < want ) {
		want = eligCount;
	}
	for ( i = 0; i < want; i++ ) {
		ballot[i] = eligible[i];
		ballotVotes[i] = 0;
	}
	ballotCount = want;
	G_MapVote_UpdateConfigString();
	G_Printf( "Map vote: %i maps on ballot\n", ballotCount );
}

void G_MapVote_Command( gentity_t *ent ) {
	char arg[MAX_TOKEN_CHARS];
	char rankArg[MAX_TOKEN_CHARS];
	int n, rank, r;

	if ( g_gametype.integer != GT_WOLF_MAPVOTE || !level.intermissiontime ) {
		return;
	}
	if ( !ent || !ent->client ) {
		return;
	}
	if ( trap_Argc() < 2 ) {
		return;
	}
	trap_Argv( 1, arg, sizeof( arg ) );
	n = atoi( arg );
	if ( n < 0 || n >= ballotCount ) {
		return;
	}

	rank = 1;
	if ( trap_Argc() >= 3 ) {
		trap_Argv( 2, rankArg, sizeof( rankArg ) );
		rank = atoi( rankArg );
	}
	if ( rank < 1 || rank > 3 ) {
		return;
	}

	for ( r = 0; r < 3; r++ ) {
		if ( r == rank - 1 ) {
			continue;
		}
		if ( ent->client->pers.mapVote[r] == n + 1 ) {
			ent->client->pers.mapVote[r] = 0;
		}
	}
	ent->client->pers.mapVote[rank - 1] = n + 1;
	G_MapVote_Recount();
	trap_SendServerCommand( ent - g_entities, va( "print \"Vote %i: %s\n\"", rank, pool[ballot[n]].longname[0] ? pool[ballot[n]].longname : pool[ballot[n]].bsp ) );
}

qboolean G_MapVote_EnoughVoted( void ) {
	int i, voted = 0, total = 0;
	gclient_t *cl;

	if ( !( g_mapVoteFlags.integer & MAPVOTE_WAIT_FOR_VOTES ) ) {
		return qtrue;
	}
	for ( i = 0; i < level.numConnectedClients; i++ ) {
		cl = level.clients + level.sortedClients[i];
		if ( cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		if ( g_entities[level.sortedClients[i]].r.svFlags & SVF_BOT ) {
			continue;
		}
		total++;
		if ( cl->pers.mapVote[0] > 0 || cl->pers.mapVote[1] > 0 || cl->pers.mapVote[2] > 0 ) {
			voted++;
		}
	}
	if ( total <= 0 ) {
		return qtrue;
	}
	return ( 100 * voted / total ) >= g_intermissionReadyPercent.value ? qtrue : qfalse;
}

const char *G_MapVote_WinningMap( void ) {
	int i, best = 0, bestVotes = -1, bestPlays, p;

	if ( g_gametype.integer != GT_WOLF_MAPVOTE || ballotCount <= 0 ) {
		return NULL;
	}

	for ( i = 0; i < ballotCount; i++ ) {
		qboolean better = qfalse;
		p = ballot[i];
		if ( ballotVotes[i] > bestVotes ) {
			better = qtrue;
		} else if ( ballotVotes[i] == bestVotes && ( g_mapVoteFlags.integer & MAPVOTE_TIE_LEASTPLAYED ) ) {
			if ( pool[p].plays < pool[ballot[best]].plays ) {
				better = qtrue;
			}
		}
		if ( better ) {
			best = i;
			bestVotes = ballotVotes[i];
			bestPlays = pool[p].plays;
		}
	}
	(void)bestPlays;
	if ( bestVotes <= 0 ) {
		return NULL;
	}
	return pool[ballot[best]].bsp;
}
