#ifndef GAME_ENH_ENH_H
#define GAME_ENH_ENH_H

#include <string>
#include <vector>

// Built-in EnhMod 1.0.9d behaviour. Formats are from the published package:
// ModEnhConfig.xml, enhmod_commands.db, enhmod_level.db, enhmod_admin.db,
// enhmod_antirush.db, forcecvarfile.cfg, commands_flags.txt, readme.txt.

void Enh_Init();
void Enh_Reload();
void Enh_Shutdown();
void Enh_ApplySpawn(struct gclient_s *client);
void Enh_RunFrame();
void Enh_ClientConnect(struct gentity_s *ent, qboolean firstTime, qboolean isBot);
void Enh_ClientBegin(struct gentity_s *ent);
void Enh_Obituary(struct gentity_s *self, struct gentity_s *attacker, int meansOfDeath);
void Enh_Damage(struct gentity_s *targ, struct gentity_s *attacker);
qboolean Enh_CallVoteAllowed(struct gentity_s *ent, const char *vote);
void Enh_RegisterBuiltins();

int Enh_AdminLevel(int clientNum);
const char *Enh_LevelName(int levelNum);
void Enh_CollectHelp(int clientNum, std::vector<std::string>& out);
void Enh_SetAdminLevel(int clientNum, int newLevel);
int Enh_LastKiller(int clientNum);
int Enh_LastKilled(int clientNum);
void Enh_SetFrozen(int clientNum, qboolean v);
qboolean Enh_IsFrozen(int clientNum);
void Enh_SetMidget(int clientNum, qboolean v);
qboolean Enh_IsMidget(int clientNum);
qboolean Enh_AddAntiRushHere(struct gentity_s *ent, const char *key, int timeout, float radius, char method);
qboolean Enh_DelAntiRush(const char *key);
void Enh_ListAntiRush(struct gentity_s *ent);
qboolean Enh_TryCustom(int clientNum, const std::vector<std::string>& args);
qboolean Enh_HasFlag(int clientNum, char flag);
void Enh_BindCountry(struct gentity_s *ent, qboolean isBot);
int Enh_CountryId(int clientNum);
const char *Enh_CountryName(int clientNum);
const char *Enh_CityName(int clientNum);
const char *Enh_PlaceName(int clientNum);

#endif
