#include <bgame/impl.h>
#include <game/enh/enh.h>
#include <cctype>

namespace cmd {

namespace {

const string& clientNamex(Client *c)
{
	if (c && connectedUsers[c->slot] && !connectedUsers[c->slot]->isNull())
		return connectedUsers[c->slot]->namex;
	static string empty;
	return empty;
}

gentity_t *entOf(Client *c)
{
	return c ? &g_entities[c->slot] : NULL;
}

AbstractCommand::PostAction warExecute(AbstractCommand::Context& txt, Cvar& cv, const char *label)
{
	if (txt._args.size() > 2)
		return AbstractCommand::PA_USAGE;

	Buffer buf;
	if (txt._args.size() == 1) {
		buf << label << " is " << xvalue(cv.ivalue ? "enabled" : "disabled") << '.';
		printChat(txt._client, buf);
		return AbstractCommand::PA_NONE;
	}

	string action = txt._args[1];
	str::toLower(action);
	if (action != "on" && action != "off") {
		txt._ebuf << "Invalid argument: " << xvalue(txt._args[1]);
		return AbstractCommand::PA_ERROR;
	}

	const bool enable = (action == "on");
	if ((enable && cv.ivalue) || (!enable && !cv.ivalue)) {
		txt._ebuf << label << " is already " << xvalue(enable ? "enabled" : "disabled") << '.';
		return AbstractCommand::PA_ERROR;
	}

	if (enable) {
		if (&cv != &cvars::g_rifleWar)
			cvars::g_rifleWar.set("0");
		if (&cv != &cvars::g_pistolWar)
			cvars::g_pistolWar.set("0");
		if (&cv != &cvars::g_pumpgunWar)
			cvars::g_pumpgunWar.set("0");
	}
	cv.set(enable ? "1" : "0");
	for (int i = 0; i < level.numConnectedClients; ++i) {
		gentity_t *ent = &g_entities[level.sortedClients[i]];
		if (!ent->client || ent->client->sess.sessionTeam == TEAM_SPECTATOR)
			continue;
		G_Damage(ent, NULL, NULL, NULL, NULL, 10000, DAMAGE_JAY_NO_PROTECTION, MOD_UNKNOWN);
	}
	buf << label << " is now " << xvalue(enable ? "enabled" : "disabled") << '.';
	printCpm(txt._client, buf, true);
	return AbstractCommand::PA_NONE;
}

} // namespace

class EnhBuiltin : public AbstractBuiltin {
protected:
	const char _enhFlag;
	EnhBuiltin(const char* name, char flag)
		: AbstractBuiltin(name), _enhFlag(flag) {}
public:
	bool hasPermission(const Context& txt) {
		if (AbstractCommand::hasPermission(txt))
			return true;
		return (txt._client && Enh_HasFlag(txt._client->slot, _enhFlag)) ? true : false;
	}
};

class RifleWar : public EnhBuiltin {
public:
	RifleWar() : EnhBuiltin("riflewar", 'r') {
		__usage << xvalue("!" + _name) << " [" << xvalue("on") << '|' << xvalue("off") << ']';
		__descr << "Rifle-only. No argument reports status.";
	}
	PostAction doExecute(Context& txt) { return warExecute(txt, cvars::g_rifleWar, "riflewar"); }
};

class PistolWar : public EnhBuiltin {
public:
	PistolWar() : EnhBuiltin("pistolwar", 'c') {
		__usage << xvalue("!" + _name) << " [" << xvalue("on") << '|' << xvalue("off") << ']';
		__descr << "Pistol-only. No argument reports status.";
	}
	PostAction doExecute(Context& txt) { return warExecute(txt, cvars::g_pistolWar, "pistolwar"); }
};

class PumpgunWar : public EnhBuiltin {
public:
	PumpgunWar() : EnhBuiltin("pumpgunwar", 'p') {
		__usage << xvalue("!" + _name) << " [" << xvalue("on") << '|' << xvalue("off") << ']';
		__descr << "M97-only. No argument reports status.";
	}
	PostAction doExecute(Context& txt) { return warExecute(txt, cvars::g_pumpgunWar, "pumpgunwar"); }
};

class Freeze : public EnhBuiltin {
public:
	Freeze() : EnhBuiltin("freeze", 'f') {
		__usage << xvalue("!" + _name) << ' ' << xvalue("PLAYER");
		__descr << "Lock a player's movement.";
	}
	PostAction doExecute(Context& txt) {
		Client *t = NULL;
		if (txt._args.size() != 2 || !lookupPLAYER(txt._args[1], txt, t))
			return txt._args.size() != 2 ? PA_USAGE : PA_ERROR;
		Enh_SetFrozen(t->slot, qtrue);
		Buffer buf;
		buf << "froze " << xvalue(clientNamex(t));
		printCpm(txt._client, buf, true);
		return PA_NONE;
	}
};

class Unfreeze : public EnhBuiltin {
public:
	Unfreeze() : EnhBuiltin("unfreeze", 'f') {
		__usage << xvalue("!" + _name) << ' ' << xvalue("PLAYER");
		__descr << "Unlock a frozen player.";
	}
	PostAction doExecute(Context& txt) {
		Client *t = NULL;
		if (txt._args.size() != 2 || !lookupPLAYER(txt._args[1], txt, t))
			return txt._args.size() != 2 ? PA_USAGE : PA_ERROR;
		Enh_SetFrozen(t->slot, qfalse);
		gentity_t *ent = entOf(t);
		if (ent && ent->client) {
			ent->client->ps.pm_flags &= ~PMF_TIME_LOCKPLAYER;
			ent->client->ps.pm_time = 0;
		}
		Buffer buf;
		buf << "unfroze " << xvalue(clientNamex(t));
		printCpm(txt._client, buf, true);
		return PA_NONE;
	}
};

class Disarm : public EnhBuiltin {
public:
	Disarm() : EnhBuiltin("disarm", 'v') {
		__usage << xvalue("!" + _name) << ' ' << xvalue("PLAYER");
		__descr << "Strip weapons; leave a knife.";
	}
	PostAction doExecute(Context& txt) {
		Client *t = NULL;
		if (txt._args.size() != 2 || !lookupPLAYER(txt._args[1], txt, t))
			return txt._args.size() != 2 ? PA_USAGE : PA_ERROR;
		gentity_t *ent = entOf(t);
		if (!ent || !ent->client)
			return PA_ERROR;
		memset(ent->client->ps.ammo, 0, sizeof(ent->client->ps.ammo));
		memset(ent->client->ps.ammoclip, 0, sizeof(ent->client->ps.ammoclip));
		ent->client->ps.weapons[0] = 0;
		ent->client->ps.weapons[1] = 0;
		AddWeaponToPlayer(ent->client, WP_KNIFE, 0, 1, qtrue);
		Buffer buf;
		buf << "disarmed " << xvalue(clientNamex(t));
		printCpm(txt._client, buf, true);
		return PA_NONE;
	}
};

class Warn : public EnhBuiltin {
public:
	Warn() : EnhBuiltin("warn", 'w') {
		__usage << xvalue("!" + _name) << ' ' << xvalue("PLAYER") << " [" << xvalue("reason") << ']';
		__descr << "Public warning.";
	}
	PostAction doExecute(Context& txt) {
		Client *t = NULL;
		if (txt._args.size() < 2 || !lookupPLAYER(txt._args[1], txt, t))
			return txt._args.size() < 2 ? PA_USAGE : PA_ERROR;
		string reason;
		for (size_t i = 2; i < txt._args.size(); ++i) {
			if (i > 2)
				reason += ' ';
			reason += txt._args[i];
		}
		if (reason.empty())
			reason = "warning";
		trap_SendServerCommand(-1, va("cpm \"^5enhmod: ^7%s ^7warned %s ^7(%s)\"",
			txt._client ? clientNamex(txt._client).c_str() : "console",
			clientNamex(t).c_str(), reason.c_str()));
		return PA_NONE;
	}
};

class Country : public EnhBuiltin {
public:
	Country() : EnhBuiltin("country", 'C') {
		__usage << xvalue("!" + _name) << " [" << xvalue("PLAYER") << ']';
		__descr << "Show a player's IP and country (GeoLite2-City or GeoLite2-Country next to qagame).";
	}
	PostAction doExecute(Context& txt) {
		Client *t = txt._client;
		if (txt._args.size() > 2)
			return PA_USAGE;
		if (txt._args.size() == 2 && !lookupPLAYER(txt._args[1], txt, t))
			return PA_ERROR;
		if (!t)
			return PA_USAGE;
		char userinfo[MAX_INFO_STRING];
		trap_GetUserinfo(t->slot, userinfo, sizeof(userinfo));
		Buffer buf;
		buf << clientNamex(t) << " ip " << xvalue(Info_ValueForKey(userinfo, "ip"))
		    << " country " << xvalue(Enh_PlaceName(t->slot));
		printChat(txt._client, buf);
		return PA_NONE;
	}
};

class Impact : public EnhBuiltin {
public:
	Impact() : EnhBuiltin("impact", 'i') {
		__usage << xvalue("!" + _name) << " [" << xvalue("PLAYER") << ']';
		__descr << "Last attacker of the player (pairs with g_drawAttackerHP).";
	}
	PostAction doExecute(Context& txt) {
		Client *t = txt._client;
		if (txt._args.size() > 2)
			return PA_USAGE;
		if (txt._args.size() == 2 && !lookupPLAYER(txt._args[1], txt, t))
			return PA_ERROR;
		if (!t)
			return PA_USAGE;
		const int k = Enh_LastKiller(t->slot);
		Buffer buf;
		if (k < 0)
			buf << clientNamex(t) << " has no last attacker.";
		else
			buf << clientNamex(t) << " last attacker " << xvalue(g_clientObjects[k].gclient.pers.netname)
			    << " hp " << xvalue(g_entities[t->slot].health);
		printChat(txt._client, buf);
		return PA_NONE;
	}
};

class CrazyDisguise : public EnhBuiltin {
public:
	CrazyDisguise() : EnhBuiltin("crazydisguise", 'd') {
		__usage << xvalue("!" + _name) << " [" << xvalue("PLAYER") << ']';
		__descr << "Give a covert-style enemy disguise.";
	}
	PostAction doExecute(Context& txt) {
		Client *t = txt._client;
		if (txt._args.size() > 2)
			return PA_USAGE;
		if (txt._args.size() == 2 && !lookupPLAYER(txt._args[1], txt, t))
			return PA_ERROR;
		gentity_t *ent = entOf(t);
		if (!ent || !ent->client)
			return PA_ERROR;
		gentity_t *src = NULL;
		for (int i = 0; i < level.numConnectedClients; ++i) {
			gentity_t *o = &g_entities[level.sortedClients[i]];
			if (!o->client || o == ent)
				continue;
			if (o->client->sess.sessionTeam == ent->client->sess.sessionTeam)
				continue;
			if (o->client->sess.sessionTeam != TEAM_AXIS && o->client->sess.sessionTeam != TEAM_ALLIES)
				continue;
			src = o;
			if ((rand() % 2) == 0)
				break;
		}
		if (!src) {
			txt._ebuf << "no enemy player to copy.";
			return PA_ERROR;
		}
		ent->client->ps.powerups[PW_OPS_DISGUISED] = 1;
		ent->client->ps.powerups[PW_OPS_CLASS_1] = src->client->sess.playerType & 1;
		ent->client->ps.powerups[PW_OPS_CLASS_2] = src->client->sess.playerType & 2;
		ent->client->ps.powerups[PW_OPS_CLASS_3] = src->client->sess.playerType & 4;
		Q_strncpyz(ent->client->disguiseNetname, src->client->pers.netname, sizeof(ent->client->disguiseNetname));
		ent->client->disguiseRank = src->client->sess.rank;
		ClientUserinfoChanged(ent - g_entities);
		Buffer buf;
		buf << "disguised " << xvalue(clientNamex(t));
		printCpm(txt._client, buf, true);
		return PA_NONE;
	}
};

class Midget : public EnhBuiltin {
public:
	Midget() : EnhBuiltin("midget", 'k') {
		__usage << xvalue("!" + _name) << " [" << xvalue("PLAYER") << ']';
		__descr << "Toggle reduced player hull.";
	}
	PostAction doExecute(Context& txt) {
		Client *t = txt._client;
		if (txt._args.size() > 2)
			return PA_USAGE;
		if (txt._args.size() == 2 && !lookupPLAYER(txt._args[1], txt, t))
			return PA_ERROR;
		gentity_t *ent = entOf(t);
		if (!ent || !ent->client)
			return PA_ERROR;
		const qboolean on = Enh_IsMidget(t->slot) ? qfalse : qtrue;
		Enh_SetMidget(t->slot, on);
		if (on) {
			ent->client->ps.viewheight = (int)(ent->client->ps.viewheight * 0.55f);
			ent->r.maxs[2] *= 0.55f;
			trap_LinkEntity(ent);
		}
		Buffer buf;
		buf << (on ? "midget on " : "midget off ") << xvalue(clientNamex(t));
		printCpm(txt._client, buf, true);
		return PA_NONE;
	}
};

class AntiRush : public EnhBuiltin {
public:
	AntiRush() : EnhBuiltin("antirush", 'x') {
		__usage << xvalue("!" + _name);
		__descr << "List antirush zones on this map (enhmod_antirush.db).";
	}
	PostAction doExecute(Context& txt) {
		Enh_ListAntiRush(entOf(txt._client));
		return PA_NONE;
	}
};

class AntiRushAdd : public EnhBuiltin {
public:
	AntiRushAdd() : EnhBuiltin("antirush_add", 'y') {
		__usage << xvalue("!" + _name) << ' ' << xvalue("KEY") << " [" << xvalue("timeout") << "] [" << xvalue("radius") << "] [r|j]";
		__descr << "Add an antirush sphere at your position. Same fields as enhmod_antirush.db.";
	}
	PostAction doExecute(Context& txt) {
		gentity_t *ent = entOf(txt._client);
		if (!ent || txt._args.size() < 2)
			return PA_USAGE;
		const int timeout = (txt._args.size() >= 3) ? atoi(txt._args[2].c_str()) : 15;
		const float radius = (txt._args.size() >= 4) ? (float)atof(txt._args[3].c_str()) : 300.f;
		char method = 'r';
		if (txt._args.size() >= 5 && !txt._args[4].empty())
			method = (char)tolower((unsigned char)txt._args[4][0]);
		if (!Enh_AddAntiRushHere(ent, txt._args[1].c_str(), timeout, radius, method))
			return PA_ERROR;
		Buffer buf;
		buf << "added antirush " << xvalue(txt._args[1]);
		printCpm(txt._client, buf, true);
		return PA_NONE;
	}
};

class AntiRushDel : public EnhBuiltin {
public:
	AntiRushDel() : EnhBuiltin("antirush_del", 'y') {
		__usage << xvalue("!" + _name) << ' ' << xvalue("KEY");
		__descr << "Remove an antirush zone on this map.";
	}
	PostAction doExecute(Context& txt) {
		if (txt._args.size() != 2)
			return PA_USAGE;
		if (!Enh_DelAntiRush(txt._args[1].c_str())) {
			txt._ebuf << "no zone named " << xvalue(txt._args[1]);
			return PA_ERROR;
		}
		Buffer buf;
		buf << "removed antirush " << xvalue(txt._args[1]);
		printCpm(txt._client, buf, true);
		return PA_NONE;
	}
};

} // namespace cmd

void Enh_RegisterBuiltins()
{
	static cmd::RifleWar      g_rifleWarCmd;
	static cmd::PistolWar     g_pistolWarCmd;
	static cmd::PumpgunWar    g_pumpgunWarCmd;
	static cmd::Freeze        g_freezeCmd;
	static cmd::Unfreeze      g_unfreezeCmd;
	static cmd::Disarm        g_disarmCmd;
	static cmd::Warn          g_warnCmd;
	static cmd::Country       g_countryCmd;
	static cmd::Impact        g_impactCmd;
	static cmd::CrazyDisguise g_crazyCmd;
	static cmd::Midget        g_midgetCmd;
	static cmd::AntiRush      g_antirushCmd;
	static cmd::AntiRushAdd   g_antirushAddCmd;
	static cmd::AntiRushDel   g_antirushDelCmd;
}
