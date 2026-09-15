#ifndef GAME_G_MAPVOTE_H
#define GAME_G_MAPVOTE_H

void G_MapVote_Init( void );
void G_MapVote_BeginIntermission( void );
void G_MapVote_Command( gentity_t *ent );
qboolean G_MapVote_EnoughVoted( void );
const char *G_MapVote_WinningMap( void );

#endif
