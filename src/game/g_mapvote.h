#ifndef GAME_G_MAPVOTE_H
#define GAME_G_MAPVOTE_H

void G_MapVote_Init( void );
void G_MapVote_BeginIntermission( void );
void G_MapVote_Command( gentity_t *ent );
qboolean G_MapVote_EnoughVoted( void );
const char *G_MapVote_WinningMap( void );
const char *G_MapVote_NextInPool( void );
/* Persist objectivecycle nextmap across voted-map detours. */
void G_MapVote_RememberCycle( const char *nextcmd );
void G_MapVote_LoadCycle( char *out, int outSize );

#endif
