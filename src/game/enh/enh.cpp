#include <bgame/impl.h>
#include <game/enh/enh.h>
#include <game/enh/enh_geoip.h>
#include <game/enh/enh_defaults.h>
#include <bgame/bg_jaymod.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;

namespace {

string trimCopy(const string& in)
{
	size_t a = 0;
	while (a < in.size() && (unsigned char)in[a] <= ' ')
		++a;
	size_t b = in.size();
	while (b > a && (unsigned char)in[b - 1] <= ' ')
		--b;
	return in.substr(a, b - a);
}

string toLowerCopy(string s)
{
	for (size_t i = 0; i < s.size(); ++i)
		s[i] = (char)tolower((unsigned char)s[i]);
	return s;
}

bool readFile(const char *name, string& out)
{
	fileHandle_t f;
	const int len = trap_FS_FOpenFile(name, &f, FS_READ);
	if (len < 0 || !f)
		return false;
	out.assign((size_t)len, '\0');
	if (len)
		trap_FS_Read(&out[0], len, f);
	trap_FS_FCloseFile(f);
	return true;
}

bool writeFile(const char *name, const string& data)
{
	fileHandle_t f;
	if (trap_FS_FOpenFile(name, &f, FS_WRITE) < 0 || !f)
		return false;
	if (!data.empty())
		trap_FS_Write(data.data(), (int)data.size(), f);
	trap_FS_FCloseFile(f);
	return true;
}

weapon_t weaponFromName(const string& raw)
{
	string n = toLowerCopy(trimCopy(raw));
	if (n.compare(0, 3, "wp_") == 0)
		n.erase(0, 3);

	static const struct { const char *n; weapon_t w; } tab[] = {
		{ "knife", WP_KNIFE },
		{ "grenade_pineapple", WP_GRENADE_PINEAPPLE },
		{ "medic_adrenaline", WP_MEDIC_ADRENALINE },
		{ "medic_syringe", WP_MEDIC_SYRINGE },
		{ "binoculars", WP_BINOCULARS },
		{ "luger", WP_LUGER },
		{ "akimbo_luger", WP_AKIMBO_LUGER },
		{ "silencer", WP_SILENCER },
		{ "akimbo_silencedluger", WP_AKIMBO_SILENCEDLUGER },
		{ "colt", WP_COLT },
		{ "akimbo_colt", WP_AKIMBO_COLT },
		{ "silenced_colt", WP_SILENCED_COLT },
		{ "akimbo_silencedcolt", WP_AKIMBO_SILENCEDCOLT },
		{ "mp40", WP_MP40 },
		{ "thompson", WP_THOMPSON },
		{ "grenade_launcher", WP_GRENADE_LAUNCHER },
		{ "panzerfaust", WP_PANZERFAUST },
		{ "flamethrower", WP_FLAMETHROWER },
		{ "mortar", WP_MORTAR },
		{ "mortar_set", WP_MORTAR_SET },
		{ "mapmortar", WP_MAPMORTAR },
		{ "mobile_mg42", WP_MOBILE_MG42 },
		{ "mobile_mg42_set", WP_MOBILE_MG42_SET },
		{ "sten", WP_STEN },
		{ "garand_scope", WP_GARAND_SCOPE },
		{ "garand", WP_GARAND },
		{ "k43", WP_K43 },
		{ "k43_scope", WP_K43_SCOPE },
		{ "fg42", WP_FG42 },
		{ "fg42scope", WP_FG42SCOPE },
		{ "kar98", WP_KAR98 },
		{ "carbine", WP_CARBINE },
		{ "gpg40", WP_GPG40 },
		{ "m7", WP_M7 },
		{ "smoke_bomb", WP_SMOKE_BOMB },
		{ "satchel", WP_SATCHEL },
		{ "satchel_det", WP_SATCHEL_DET },
		{ "poisonsyringe", WP_POISON_SYRINGE },
		{ "poison_syringe", WP_POISON_SYRINGE },
		{ "m97", WP_M97 },
		{ "poison_bomb", WP_POISON_GAS },
		{ "landpoison", WP_LANDMINE_PGAS },
		{ "molotov", WP_MOLOTOV },
		{ 0, WP_NONE }
	};
	for (int i = 0; tab[i].n; ++i) {
		if (n == tab[i].n)
			return tab[i].w;
	}
	return WP_NONE;
}

int classFromName(const string& raw)
{
	const string n = toLowerCopy(trimCopy(raw));
	if (n.empty() || n == "*")
		return -1;
	if (n == "soldier")
		return PC_SOLDIER;
	if (n == "medic")
		return PC_MEDIC;
	if (n == "engineer")
		return PC_ENGINEER;
	if (n == "fieldop" || n == "fieldops")
		return PC_FIELDOPS;
	if (n == "covertop" || n == "covertops")
		return PC_COVERTOPS;
	return -2;
}

int teamFromName(const string& raw)
{
	const string n = toLowerCopy(trimCopy(raw));
	if (n.empty() || n == "*")
		return -1;
	if (n == "axis")
		return TEAM_AXIS;
	if (n == "allies")
		return TEAM_ALLIES;
	return -2;
}

string attrValue(const string& tag, const char *key)
{
	string pat = string(key) + "=\"";
	string lower = toLowerCopy(tag);
	string lkey = toLowerCopy(pat);
	size_t p = lower.find(lkey);
	if (p == string::npos) {
		pat = string(key) + "='";
		lkey = toLowerCopy(pat);
		p = lower.find(lkey);
		if (p == string::npos)
			return "";
		size_t start = p + pat.size();
		size_t end = tag.find('\'', start);
		if (end == string::npos)
			return "";
		return tag.substr(start, end - start);
	}
	size_t start = p + pat.size();
	size_t end = tag.find('"', start);
	if (end == string::npos)
		return "";
	return tag.substr(start, end - start);
}

string childText(const string& body, const char *tag)
{
	string open = string("<") + tag + ">";
	string close = string("</") + tag + ">";
	string lbody = toLowerCopy(body);
	string lopen = toLowerCopy(open);
	string lclose = toLowerCopy(close);
	size_t a = lbody.find(lopen);
	if (a == string::npos)
		return "";
	a += open.size();
	size_t b = lbody.find(lclose, a);
	if (b == string::npos)
		return "";
	return trimCopy(body.substr(a, b - a));
}

struct XmlEntity {
	string method;
	int cls;
	int team;
	weapon_t ifWeapon;
	weapon_t giveWeapon;
	int ammo;
	int ammoclip;
	qboolean hold;
};

struct CommonCfg {
	int multikillSeconds;
	qboolean modifiedDoubleJump;
	qboolean advancedPlayerInfo;
};

struct CustomCmd {
	string name;
	string desc;
	string exec;
	vector<int> levels;
};

struct EnhLevel {
	int level;
	string name;
	string flags;
};

struct EnhAdmin {
	string name;
	string guid;
	int level;
	string flags;
};

struct AntiRushZone {
	string map;
	string key;
	int team;
	vec3_t origin;
	float radius;
	int timeout;
	char method;
};

struct ForceRule {
	qboolean ex;
	string cvar;
	string op;
	string a;
	string b;
};

CommonCfg g_common;
vector<XmlEntity> g_entitiesXml;
vector<CustomCmd> g_custom;
vector<EnhLevel> g_levels;
vector<EnhAdmin> g_admins;
vector<AntiRushZone> g_zones;
vector<ForceRule> g_force;

int g_lastKiller[MAX_CLIENTS];
int g_lastKilled[MAX_CLIENTS];
int g_killStreak[MAX_CLIENTS];
int g_killStreakTime[MAX_CLIENTS];
qboolean g_frozen[MAX_CLIENTS];
qboolean g_midget[MAX_CLIENTS];
qboolean g_firstHeadshot;
int g_antirushWarned[MAX_CLIENTS];
int g_countryId[MAX_CLIENTS];
char g_cityName[MAX_CLIENTS][64];
qboolean g_needConnectInfo[MAX_CLIENTS];

void resetRuntime()
{
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		g_lastKiller[i] = -1;
		g_lastKilled[i] = -1;
		g_killStreak[i] = 0;
		g_killStreakTime[i] = 0;
		g_frozen[i] = qfalse;
		g_midget[i] = qfalse;
		g_antirushWarned[i] = 0;
		g_countryId[i] = 0;
		g_cityName[i][0] = 0;
		g_needConnectInfo[i] = qfalse;
	}
	g_firstHeadshot = qfalse;
}

void parseCommon(const string& xml)
{
	g_common.multikillSeconds = -1;
	g_common.modifiedDoubleJump = qfalse;
	g_common.advancedPlayerInfo = qtrue;

	string mk = childText(xml, "multikilldetection");
	if (!mk.empty())
		g_common.multikillSeconds = atoi(mk.c_str());
	string dj = toLowerCopy(childText(xml, "modifieddoublejump"));
	g_common.modifiedDoubleJump = (dj == "true" || dj == "1") ? qtrue : qfalse;
	string api = toLowerCopy(childText(xml, "advancedplayerinfo"));
	if (!api.empty())
		g_common.advancedPlayerInfo = (api == "true" || api == "1") ? qtrue : qfalse;

	if (g_common.modifiedDoubleJump) {
		const int v = cvars::bg_misc.ivalue | MISC_DOUBLEJUMP;
		cvars::bg_misc.set(va("%d", v));
	}
}

void parseEntities(const string& xml)
{
	g_entitiesXml.clear();
	string lxml = toLowerCopy(xml);
	size_t pos = 0;
	for (;;) {
		size_t a = lxml.find("<entity", pos);
		if (a == string::npos)
			break;
		size_t tagEnd = xml.find('>', a);
		if (tagEnd == string::npos)
			break;
		size_t b = lxml.find("</entity>", tagEnd);
		if (b == string::npos)
			break;
		const string tag = xml.substr(a, tagEnd - a + 1);
		const string body = xml.substr(tagEnd + 1, b - (tagEnd + 1));

		XmlEntity e;
		e.method = toLowerCopy(attrValue(tag, "method"));
		e.cls = classFromName(attrValue(tag, "class"));
		e.team = teamFromName(attrValue(tag, "team"));
		e.ifWeapon = weaponFromName(attrValue(tag, "weapon"));
		e.giveWeapon = weaponFromName(childText(body, "weapon"));
		if (e.giveWeapon == WP_NONE)
			e.giveWeapon = e.ifWeapon;
		e.ammo = atoi(childText(body, "ammo").c_str());
		e.ammoclip = atoi(childText(body, "ammoclip").c_str());
		const string hold = toLowerCopy(childText(body, "hold"));
		e.hold = (hold == "true" || hold == "1") ? qtrue : qfalse;
		if (e.cls != -2 && e.team != -2 && (e.method == "add" || e.method == "if" || e.method == "remove"))
			g_entitiesXml.push_back(e);
		pos = b + 9;
	}
}

int skillFromName(const string& raw)
{
	string n = toLowerCopy(trimCopy(raw));
	if (n.empty())
		return -1;
	if (n == "light_weapons" || n == "lightweapons" || n == "lw")
		return SK_LIGHT_WEAPONS;
	if (n == "engineer" || n == "explosives" || n == "explosives_and_construction")
		return SK_EXPLOSIVES_AND_CONSTRUCTION;
	if (n == "battle_sense" || n == "battlesense")
		return SK_BATTLE_SENSE;
	if (n == "medic" || n == "first_aid" || n == "firstaid")
		return SK_FIRST_AID;
	if (n == "fieldops" || n == "field_ops" || n == "signals")
		return SK_SIGNALS;
	if (n == "soldier" || n == "heavy_weapons" || n == "heavyweapons")
		return SK_HEAVY_WEAPONS;
	if (n == "covertops" || n == "covert_ops" || n == "scoped" || n == "military_intelligence")
		return SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS;
	return -1;
}

string xmlBlock(const string& xml, const char *tag)
{
	string lxml = toLowerCopy(xml);
	string open = string("<") + tag;
	string close = string("</") + tag + ">";
	size_t a = lxml.find(toLowerCopy(open));
	if (a == string::npos)
		return "";
	size_t tagEnd = xml.find('>', a);
	if (tagEnd == string::npos)
		return "";
	size_t b = lxml.find(toLowerCopy(close), tagEnd);
	if (b == string::npos)
		return "";
	return xml.substr(tagEnd + 1, b - (tagEnd + 1));
}

int attrInt(const string& tag, const char *key, int missing)
{
	string v = attrValue(tag, key);
	if (v.empty())
		return missing;
	return atoi(v.c_str());
}

int parseXpAttr(const string& tag)
{
	string v = toLowerCopy(attrValue(tag, "xp"));
	if (v.empty())
		return -1;
	if (v == "max")
		return BG_AMMO_XP_MAX;
	return atoi(v.c_str());
}

string stripXmlComments(string xml)
{
	for (;;) {
		size_t a = xml.find("<!--");
		if (a == string::npos)
			break;
		size_t b = xml.find("-->", a);
		if (b == string::npos) {
			xml.erase(a);
			break;
		}
		xml.erase(a, b + 3 - a);
	}
	return xml;
}

void addShorthandAmmoTiers(weapon_t w, const string& tag)
{
	weaponAmmoTier_t base;
	weaponAmmoTier_t gated;
	const int clip = attrInt(tag, "maxclip", -1);
	const int ammo = attrInt(tag, "maxammo", -1);
	const int ammoS = attrInt(tag, "maxammo_skilled", -1);
	const int sk = skillFromName(attrValue(tag, "skill"));
	const int sk2 = skillFromName(attrValue(tag, "skill2"));
	const int xp = parseXpAttr(tag);

	if (clip < 0 && ammo < 0 && ammoS < 0)
		return;

	memset(&base, 0, sizeof(base));
	base.maxclip = clip;
	base.maxammo = ammo;
	BG_AddWeaponAmmoTier(w, &base);

	if (ammoS < 0 || (sk < 0 && sk2 < 0))
		return;

	memset(&gated, 0, sizeof(gated));
	gated.maxclip = clip;
	gated.maxammo = ammoS;
	gated.nNeeds = 1;
	if (sk >= 0) {
		gated.skill[0] = sk;
		gated.xp[0] = xp;
		BG_AddWeaponAmmoTier(w, &gated);
	}
	if (sk2 >= 0) {
		gated.skill[0] = sk2;
		gated.xp[0] = xp;
		BG_AddWeaponAmmoTier(w, &gated);
	}
}

void parseAmmoNeeds(const string& inner, weaponAmmoTier_t *t)
{
	string linner = toLowerCopy(inner);
	size_t pos = 0;

	while (t->nNeeds < BG_AMMO_MAX_NEEDS) {
		size_t a = linner.find("<need", pos);
		if (a == string::npos)
			break;
		size_t tagEnd = inner.find('>', a);
		if (tagEnd == string::npos)
			break;
		const string tag = inner.substr(a, tagEnd - a + 1);
		const int sk = skillFromName(attrValue(tag, "skill"));
		if (sk >= 0) {
			t->skill[t->nNeeds] = sk;
			t->xp[t->nNeeds] = parseXpAttr(tag);
			t->nNeeds++;
		}
		pos = tagEnd + 1;
	}
}

int parseAmmoTiers(weapon_t w, const string& inner)
{
	string linner = toLowerCopy(inner);
	size_t pos = 0;
	int added = 0;

	for (;;) {
		weaponAmmoTier_t t;
		size_t a = linner.find("<tier", pos);
		size_t tagEnd;
		string tag;
		qboolean selfClose;

		if (a == string::npos)
			break;
		tagEnd = inner.find('>', a);
		if (tagEnd == string::npos)
			break;
		tag = inner.substr(a, tagEnd - a + 1);
		memset(&t, 0, sizeof(t));
		t.maxclip = attrInt(tag, "maxclip", -1);
		t.maxammo = attrInt(tag, "maxammo", -1);
		selfClose = (tag.size() >= 2 && tag[tag.size() - 2] == '/') ? qtrue : qfalse;
		if (!selfClose) {
			size_t b = linner.find("</tier>", tagEnd);
			if (b != string::npos)
				parseAmmoNeeds(inner.substr(tagEnd + 1, b - (tagEnd + 1)), &t);
			pos = (b == string::npos) ? tagEnd + 1 : b + 7;
		} else {
			pos = tagEnd + 1;
		}
		if (BG_AddWeaponAmmoTier(w, &t))
			++added;
	}
	return added;
}

void parseWeaponAmmo(const string& xml)
{
	BG_ClearWeaponAmmoOverrides();

	string body = xmlBlock(stripXmlComments(xml), "weaponammo");
	if (body.empty()) {
		trap_SetConfigstring(CS_WEAPONAMMO, "");
		return;
	}

	string lbody = toLowerCopy(body);
	size_t pos = 0;
	int count = 0;
	for (;;) {
		size_t a = lbody.find("<weapon", pos);
		size_t tagEnd;
		string tag;
		string inner;
		weapon_t w;
		qboolean selfClose;

		if (a == string::npos)
			break;
		if (lbody.compare(a, 10, "<weaponamm") == 0) {
			pos = a + 7;
			continue;
		}
		tagEnd = body.find('>', a);
		if (tagEnd == string::npos)
			break;
		tag = body.substr(a, tagEnd - a + 1);
		selfClose = (tag.size() >= 2 && tag[tag.size() - 2] == '/') ? qtrue : qfalse;
		if (!selfClose) {
			size_t b = lbody.find("</weapon>", tagEnd);
			if (b != string::npos) {
				inner = body.substr(tagEnd + 1, b - (tagEnd + 1));
				pos = b + 9;
			} else {
				pos = tagEnd + 1;
			}
		} else {
			pos = tagEnd + 1;
		}

		w = weaponFromName(attrValue(tag, "name"));
		if (w == WP_NONE)
			continue;
		if (parseAmmoTiers(w, inner) <= 0)
			addShorthandAmmoTiers(w, tag);
		if (bg_weaponAmmoOverride[w].used)
			++count;
	}

	char cs[MAX_STRING_CHARS];
	BG_WriteWeaponAmmoConfig(cs, sizeof(cs));
	trap_SetConfigstring(CS_WEAPONAMMO, cs);
	ammoTableNeedsUpdate = true;
	BG_updateAmmoTable();
	G_Printf("ENHMOD: %d weapon ammo overrides from ModEnhConfig.xml\n", count);
}

void parseIniBlocks(const string& text, const char *block, vector<map<string, string> >& out)
{
	out.clear();
	map<string, string> cur;
	qboolean in = qfalse;
	const string want = string("[") + block + "]";
	std::istringstream inStream(text);
	string line;
	while (std::getline(inStream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		string t = trimCopy(line);
		if (t.empty() || t[0] == '#' || t[0] == ';')
			continue;
		if (t[0] == '[') {
			if (in && !cur.empty())
				out.push_back(cur);
			cur.clear();
			in = (toLowerCopy(t) == want) ? qtrue : qfalse;
			continue;
		}
		if (!in)
			continue;
		size_t eq = t.find('=');
		if (eq == string::npos)
			continue;
		cur[toLowerCopy(trimCopy(t.substr(0, eq)))] = trimCopy(t.substr(eq + 1));
	}
	if (in && !cur.empty())
		out.push_back(cur);
}

void loadCommands(const string& text)
{
	g_custom.clear();
	vector<map<string, string> > blocks;
	parseIniBlocks(text, "command", blocks);
	for (size_t i = 0; i < blocks.size(); ++i) {
		CustomCmd c;
		c.name = toLowerCopy(blocks[i]["command"]);
		c.desc = blocks[i]["desc"];
		c.exec = blocks[i]["exec"];
		std::istringstream ls(blocks[i]["levels"]);
		int lv;
		while (ls >> lv)
			c.levels.push_back(lv);
		if (!c.name.empty() && !c.exec.empty())
			g_custom.push_back(c);
	}
}

void loadLevels(const string& text)
{
	g_levels.clear();
	vector<map<string, string> > blocks;
	parseIniBlocks(text, "level", blocks);
	for (size_t i = 0; i < blocks.size(); ++i) {
		EnhLevel lv;
		lv.level = atoi(blocks[i]["level"].c_str());
		lv.name = blocks[i]["name"];
		lv.flags = blocks[i]["flags"];
		g_levels.push_back(lv);
	}
}

void loadAdmins(const string& text)
{
	g_admins.clear();
	vector<map<string, string> > blocks;
	parseIniBlocks(text, "admin", blocks);
	for (size_t i = 0; i < blocks.size(); ++i) {
		EnhAdmin a;
		a.name = blocks[i]["name"];
		a.guid = toLowerCopy(blocks[i]["guid"]);
		a.level = atoi(blocks[i]["level"].c_str());
		a.flags = blocks[i]["flags"];
		if (!a.guid.empty())
			g_admins.push_back(a);
	}
}

void loadZones(const string& text)
{
	g_zones.clear();
	vector<map<string, string> > blocks;
	parseIniBlocks(text, "map", blocks);
	for (size_t i = 0; i < blocks.size(); ++i) {
		AntiRushZone z;
		memset(&z, 0, sizeof(z));
		z.map = toLowerCopy(blocks[i]["name"]);
		z.key = blocks[i]["key"];
		z.team = (int)atof(blocks[i]["team"].c_str());
		z.origin[0] = (float)atof(blocks[i]["coordx"].c_str());
		z.origin[1] = (float)atof(blocks[i]["coordy"].c_str());
		z.origin[2] = (float)atof(blocks[i]["coordz"].c_str());
		z.radius = (float)atof(blocks[i]["radius"].c_str());
		z.timeout = atoi(blocks[i]["timeout"].c_str());
		const string m = toLowerCopy(blocks[i]["method"]);
		z.method = m.empty() ? 'r' : m[0];
		if (!z.map.empty() && z.radius > 0)
			g_zones.push_back(z);
	}
}

void loadForce(const string& text)
{
	g_force.clear();
	std::istringstream in(text);
	string line;
	while (std::getline(in, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		string t = trimCopy(line);
		if (t.empty() || t[0] == '#' || t[0] == ';')
			continue;
		std::istringstream ls(t);
		string kind;
		ls >> kind;
		kind = toLowerCopy(kind);
		ForceRule r;
		r.ex = qfalse;
		r.op = "eq";
		if (kind == "forcecvar") {
			ls >> r.cvar >> r.a;
			if (!r.cvar.empty())
				g_force.push_back(r);
		} else if (kind == "forcecvarex") {
			r.ex = qtrue;
			ls >> r.cvar >> r.op >> r.a >> r.b;
			r.op = toLowerCopy(r.op);
			if (!r.cvar.empty())
				g_force.push_back(r);
		}
	}
}

string enhMapName()
{
	return toLowerCopy(level.rawmapname);
}

void teleportTeamSpawn(gentity_t *ent)
{
	vec3_t origin, angles;
	if (!SelectCTFSpawnPoint(ent->client->sess.sessionTeam,
			ent->client->pers.teamState.state,
			origin, angles,
			ent->client->sess.spawnObjectiveIndex))
		return;
	TeleportPlayer(ent, origin, angles);
}

void applyForceCvars(int clientNum)
{
	for (size_t i = 0; i < g_force.size(); ++i) {
		const ForceRule& r = g_force[i];
		if (!r.ex) {
			trap_SendServerCommand(clientNum, va("forcecvar %s %s", r.cvar.c_str(), r.a.c_str()));
			continue;
		}
		// forcecvarex is evaluated when the client reports the cvar.
		// The published file still lists the rule; send the bounds as forcecvar
		// of the low (IN) or exact (EQ) value so a connecting client is clamped.
		if (r.op == "eq" || r.op == "equal")
			trap_SendServerCommand(clientNum, va("forcecvar %s %s", r.cvar.c_str(), r.a.c_str()));
		else if (r.op == "in" || r.op == "inside")
			trap_SendServerCommand(clientNum, va("forcecvar %s %s", r.cvar.c_str(), r.a.c_str()));
	}
}

string maskIp(const char *ip)
{
	string s = ip ? ip : "";
	size_t colon = s.find(':');
	string port;
	if (colon != string::npos) {
		port = s.substr(colon);
		s.erase(colon);
	}
	int dots = 0;
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == '.') {
			++dots;
			if (dots >= 2) {
				s.erase(i + 1);
				s += "***.***";
				break;
			}
		}
	}
	(void)port;
	return s;
}

void fixAkimbo(gclient_t *client)
{
	static const weapon_t akimbos[] = {
		WP_AKIMBO_COLT,
		WP_AKIMBO_LUGER,
		WP_AKIMBO_SILENCEDCOLT,
		WP_AKIMBO_SILENCEDLUGER
	};
	for (int i = 0; i < 4; ++i) {
		const weapon_t w = akimbos[i];
		if (!COM_BitCheck(client->ps.weapons, w))
			continue;
		const weapon_t side = (weapon_t)BG_AkimboSidearm(w);
		if (side == WP_NONE)
			continue;
		client->ps.ammoclip[BG_FindClipForWeapon(side)] =
			client->ps.ammoclip[BG_FindClipForWeapon(w)];
	}
}

int classBit(int pc)
{
	switch (pc) {
	case PC_SOLDIER: return 1;
	case PC_MEDIC: return 2;
	case PC_ENGINEER: return 4;
	case PC_FIELDOPS: return 8;
	case PC_COVERTOPS: return 16;
	default: return 0;
	}
}

void applyAdrenalineCls(gclient_t *client)
{
	const int bits = cvars::g_adrenenalinecls.ivalue;
	const int mine = classBit(client->sess.playerType);
	if (mine && !(bits & mine)) {
		COM_BitClear(client->ps.weapons, WP_MEDIC_ADRENALINE);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_MEDIC_ADRENALINE)] = 0;
		return;
	}
	if (mine && (bits & mine) && !COM_BitCheck(client->ps.weapons, WP_MEDIC_ADRENALINE))
		AddWeaponToPlayer(client, WP_MEDIC_ADRENALINE, 0, 10, qfalse);
}

qboolean applyWarLoadout(gclient_t *client)
{
	weapon_t w = WP_NONE;
	if (cvars::g_rifleWar.ivalue)
		w = (client->sess.sessionTeam == TEAM_AXIS) ? WP_KAR98 : WP_CARBINE;
	else if (cvars::g_pistolWar.ivalue)
		w = (client->sess.sessionTeam == TEAM_AXIS) ? WP_LUGER : WP_COLT;
	else if (cvars::g_pumpgunWar.ivalue)
		w = WP_M97;
	if (w == WP_NONE)
		return qfalse;

	memset(client->ps.weapons, 0, sizeof(client->ps.weapons));
	memset(client->ps.ammo, 0, sizeof(client->ps.ammo));
	memset(client->ps.ammoclip, 0, sizeof(client->ps.ammoclip));

	AddWeaponToPlayer(client, WP_KNIFE, 0, 1, qfalse);
	AddWeaponToPlayer(client, w, 400, GetAmmoTableData(w)->defaultStartingClip, qtrue);
	if (w == WP_KAR98)
		AddWeaponToPlayer(client, WP_GPG40, 8, 1, qfalse);
	if (w == WP_CARBINE)
		AddWeaponToPlayer(client, WP_M7, 8, 1, qfalse);
	G_AddClassSpecificTools(client);
	return qtrue;
}

void applyXml(gclient_t *client)
{
	const int pc = client->sess.playerType;
	const int team = client->sess.sessionTeam;
	for (size_t i = 0; i < g_entitiesXml.size(); ++i) {
		const XmlEntity& e = g_entitiesXml[i];
		if (e.cls >= 0 && e.cls != pc)
			continue;
		if (e.team >= 0 && e.team != team)
			continue;

		if (e.method == "if") {
			if (e.ifWeapon == WP_NONE || !COM_BitCheck(client->ps.weapons, e.ifWeapon))
				continue;
			if (e.giveWeapon == WP_NONE)
				continue;
			AddWeaponToPlayer(client, e.giveWeapon, e.ammo, e.ammoclip, e.hold);
		} else if (e.method == "add") {
			if (e.giveWeapon == WP_NONE)
				continue;
			AddWeaponToPlayer(client, e.giveWeapon, e.ammo, e.ammoclip, e.hold);
		} else if (e.method == "remove") {
			const weapon_t w = (e.ifWeapon != WP_NONE) ? e.ifWeapon : e.giveWeapon;
			if (w == WP_NONE)
				continue;
			COM_BitClear(client->ps.weapons, w);
			client->ps.ammo[BG_FindAmmoForWeapon(w)] = 0;
			client->ps.ammoclip[BG_FindClipForWeapon(w)] = 0;
		}
	}
}

} // namespace

void saveAdmins()
{
	std::ostringstream os;
	for (size_t i = 0; i < g_admins.size(); ++i) {
		const EnhAdmin& a = g_admins[i];
		os << "[admin]\n"
		   << "name    = " << a.name << "\n"
		   << "guid    = " << a.guid << "\n"
		   << "level   = " << a.level << "\n"
		   << "flags   = " << a.flags << "\n\n";
	}
	if (!writeFile("enhmod_admin.db", os.str()))
		G_Printf("ENHMOD: could not write enhmod_admin.db\n");
}

void loadEnhFiles()
{
	string xml;
	if (readFile("ModEnhConfig.xml", xml)) {
		parseCommon(xml);
		parseEntities(xml);
		parseWeaponAmmo(xml);
		G_Printf("ENHMOD: ModEnhConfig.xml loaded (%d entity rules)\n", (int)g_entitiesXml.size());
	} else {
		BG_ClearWeaponAmmoOverrides();
		trap_SetConfigstring(CS_WEAPONAMMO, "");
	}

	string text;
	if (readFile("enhmod_commands.db", text)) {
		loadCommands(text);
		G_Printf("ENHMOD: enhmod_commands.db loaded (%d commands)\n", (int)g_custom.size());
	} else {
		g_custom.clear();
	}
	if (readFile("enhmod_level.db", text)) {
		loadLevels(text);
		G_Printf("ENHMOD: enhmod_level.db loaded (%d levels)\n", (int)g_levels.size());
	} else {
		g_levels.clear();
		G_Printf("ENHMOD: enhmod_level.db missing\n");
	}
	if (readFile("enhmod_admin.db", text)) {
		loadAdmins(text);
		G_Printf("ENHMOD: enhmod_admin.db loaded (%d admins)\n", (int)g_admins.size());
	} else {
		g_admins.clear();
		G_Printf("ENHMOD: enhmod_admin.db missing\n");
	}
	if (readFile("enhmod_antirush.db", text)) {
		loadZones(text);
		G_Printf("ENHMOD: enhmod_antirush.db loaded (%d zones)\n", (int)g_zones.size());
	}
	if (readFile("forcecvarfile.cfg", text))
		loadForce(text);
}

qboolean Enh_LevelAllowed(int clientNum, const vector<int>& levels);

const char *Enh_LevelName(int levelNum)
{
	for (size_t i = 0; i < g_levels.size(); ++i) {
		if (g_levels[i].level == levelNum)
			return g_levels[i].name.c_str();
	}
	return "";
}

void Enh_CollectHelp(int clientNum, vector<string>& out)
{
	static const struct {
		const char *name;
		char flag;
	} kBuiltins[] = {
		{ "dbload", 'G' },
		{ "help", 'h' },
		{ "admintest", 'a' },
		{ "setlevel", 's' },
		{ "crazydisguise", 'd' },
		{ "riflewar", 'r' },
		{ "pistolwar", 'c' },
		{ "country", 'C' },
		{ "freeze", 'f' },
		{ "unfreeze", 'f' },
		{ "disarm", 'v' },
		{ "warn", 'w' },
		{ "impact", 'i' },
		{ "antirush", 'x' },
		{ "antirush_add", 'y' },
		{ "antirush_del", 'y' },
		{ "midget", 'k' },
		{ "pumpgunwar", 'p' },
	};
	out.clear();
	for (size_t i = 0; i < sizeof(kBuiltins) / sizeof(kBuiltins[0]); ++i) {
		if (Enh_HasFlag(clientNum, kBuiltins[i].flag))
			out.push_back(kBuiltins[i].name);
	}
	for (size_t i = 0; i < g_custom.size(); ++i) {
		if (Enh_LevelAllowed(clientNum, g_custom[i].levels))
			out.push_back(g_custom[i].name);
	}
}

void Enh_SetAdminLevel(int clientNum, int newLevel)
{
	if (clientNum < 0 || clientNum >= MAX_CLIENTS || !connectedUsers[clientNum])
		return;
	const string guid = toLowerCopy(connectedUsers[clientNum]->guid);
	if (guid.empty())
		return;
	string name = connectedUsers[clientNum]->namex;
	for (size_t i = 0; i < g_admins.size(); ++i) {
		if (g_admins[i].guid == guid) {
			g_admins[i].name = name;
			g_admins[i].level = newLevel;
			saveAdmins();
			return;
		}
	}
	EnhAdmin a;
	a.name = name;
	a.guid = guid;
	a.level = newLevel;
	g_admins.push_back(a);
	saveAdmins();
}

int Enh_AdminLevel(int clientNum)
{
	if (clientNum < 0 || clientNum >= MAX_CLIENTS || !connectedUsers[clientNum])
		return 0;
	string guid = toLowerCopy(connectedUsers[clientNum]->guid);
	for (size_t i = 0; i < g_admins.size(); ++i) {
		if (g_admins[i].guid == guid)
			return g_admins[i].level;
	}
	return connectedUsers[clientNum]->authLevel;
}

qboolean Enh_HasFlag(int clientNum, char flag)
{
	if (clientNum < 0 || clientNum >= MAX_CLIENTS || !connectedUsers[clientNum] || !flag)
		return qfalse;
	string guid = toLowerCopy(connectedUsers[clientNum]->guid);
	string flags;
	for (size_t i = 0; i < g_admins.size(); ++i) {
		if (g_admins[i].guid == guid) {
			flags = g_admins[i].flags;
			break;
		}
	}
	if (flags.empty()) {
		const int level = Enh_AdminLevel(clientNum);
		for (size_t i = 0; i < g_levels.size(); ++i) {
			if (g_levels[i].level == level)
				flags = g_levels[i].flags;
		}
	}
	return (flags.find(flag) != string::npos) ? qtrue : qfalse;
}

void Enh_SaveAntiRush()
{
	std::ostringstream os;
	for (size_t i = 0; i < g_zones.size(); ++i) {
		const AntiRushZone& z = g_zones[i];
		os << "[map]\n"
		   << "name    = " << z.map << "\n"
		   << "key     = " << z.key << "\n"
		   << "team    = " << z.team << ".000000\n"
		   << "coordx  = " << z.origin[0] << "\n"
		   << "coordy  = " << z.origin[1] << "\n"
		   << "coordz  = " << z.origin[2] << "\n"
		   << "radius  = " << (int)z.radius << "\n"
		   << "timeout = " << z.timeout << "\n"
		   << "method  = " << z.method << "\n\n";
	}
	writeFile("enhmod_antirush.db", os.str());
}

const vector<CustomCmd>& Enh_CustomCommands()
{
	return g_custom;
}

qboolean Enh_LevelAllowed(int clientNum, const vector<int>& levels)
{
	const int lv = Enh_AdminLevel(clientNum);
	if (levels.empty())
		return qtrue;
	for (size_t i = 0; i < levels.size(); ++i) {
		if (levels[i] == lv)
			return qtrue;
	}
	return qfalse;
}

int Enh_LastKiller(int clientNum) { return (clientNum >= 0 && clientNum < MAX_CLIENTS) ? g_lastKiller[clientNum] : -1; }
int Enh_LastKilled(int clientNum) { return (clientNum >= 0 && clientNum < MAX_CLIENTS) ? g_lastKilled[clientNum] : -1; }
qboolean Enh_IsFrozen(int clientNum) { return (clientNum >= 0 && clientNum < MAX_CLIENTS) ? g_frozen[clientNum] : qfalse; }
void Enh_SetFrozen(int clientNum, qboolean v) { if (clientNum >= 0 && clientNum < MAX_CLIENTS) g_frozen[clientNum] = v; }
qboolean Enh_IsMidget(int clientNum) { return (clientNum >= 0 && clientNum < MAX_CLIENTS) ? g_midget[clientNum] : qfalse; }
void Enh_SetMidget(int clientNum, qboolean v) { if (clientNum >= 0 && clientNum < MAX_CLIENTS) g_midget[clientNum] = v; }

void Enh_AddZone(const AntiRushZone& z) { g_zones.push_back(z); Enh_SaveAntiRush(); }
qboolean Enh_DelZone(const string& map, const string& key)
{
	qboolean hit = qfalse;
	vector<AntiRushZone> keep;
	for (size_t i = 0; i < g_zones.size(); ++i) {
		if (g_zones[i].map == toLowerCopy(map) && toLowerCopy(g_zones[i].key) == toLowerCopy(key)) {
			hit = qtrue;
			continue;
		}
		keep.push_back(g_zones[i]);
	}
	g_zones.swap(keep);
	if (hit)
		Enh_SaveAntiRush();
	return hit;
}

const vector<AntiRushZone>& Enh_Zones() { return g_zones; }

void seedIfMissing(const char *name, const char *data)
{
	string unused;
	if (readFile(name, unused))
		return;
	if (writeFile(name, data ? data : ""))
		G_Printf("ENHMOD: created default %s\n", name);
	else
		G_Printf("ENHMOD: could not create %s\n", name);
}

void Enh_Init()
{
	resetRuntime();
	g_entitiesXml.clear();
	g_custom.clear();
	g_levels.clear();
	g_admins.clear();
	g_zones.clear();
	g_force.clear();
	g_common.multikillSeconds = -1;
	g_common.modifiedDoubleJump = qfalse;
	g_common.advancedPlayerInfo = qtrue;

	seedIfMissing("ModEnhConfig.xml", EnhDef_Xml());
	seedIfMissing("enhmod_commands.db", EnhDef_Commands());
	seedIfMissing("enhmod_level.db", EnhDef_Levels());
	seedIfMissing("enhmod_admin.db", EnhDef_Admin());
	seedIfMissing("enhmod_antirush.db", EnhDef_Zones());
	seedIfMissing("forcecvarfile.cfg", EnhDef_Force());
	seedIfMissing("commands_flags.txt", EnhDef_Flags());

	loadEnhFiles();
	EnhGeo_Init();
	Enh_RegisterBuiltins();
}

void Enh_Reload()
{
	loadEnhFiles();
	EnhGeo_Init();
}

void Enh_Shutdown()
{
	EnhGeo_Shutdown();
}

void Enh_ApplySpawn(gclient_t *client)
{
	if (!client)
		return;
	if (!applyWarLoadout(client))
		applyXml(client);
	applyAdrenalineCls(client);
	fixAkimbo(client);
	if (Enh_IsMidget(client->ps.clientNum)) {
		client->ps.viewheight = (int)(client->ps.viewheight * 0.55f);
		client->ps.mins[2] = client->ps.mins[2] * 0.55f;
		client->ps.maxs[2] = client->ps.maxs[2] * 0.55f;
	}
}

void Enh_RunFrame()
{
	const string map = enhMapName();
	for (int i = 0; i < level.numConnectedClients; ++i) {
		gentity_t *ent = &g_entities[level.sortedClients[i]];
		if (!ent->inuse || !ent->client)
			continue;
		const int n = ent - g_entities;
		if (Enh_IsFrozen(n) && ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
			ent->client->ps.pm_flags |= PMF_TIME_LOCKPLAYER;
			ent->client->ps.pm_time = 100;
			VectorClear(ent->client->ps.velocity);
		}
		if (ent->health <= 0 || ent->client->sess.sessionTeam == TEAM_SPECTATOR)
			continue;
		for (size_t z = 0; z < g_zones.size(); ++z) {
			const AntiRushZone& zone = g_zones[z];
			if (zone.map != map)
				continue;
			if (zone.team && zone.team != ent->client->sess.sessionTeam)
				continue;
			if (level.time >= level.startTime + zone.timeout * 1000)
				continue;
			vec3_t d;
			VectorSubtract(ent->r.currentOrigin, zone.origin, d);
			if (VectorLength(d) > zone.radius)
				continue;
			teleportTeamSpawn(ent);
			if (level.time >= g_antirushWarned[n]) {
				trap_SendServerCommand(n, va("cpm \"^5enhmod: ^7antirush (%s) %d sec remaining\"",
					zone.key.c_str(), (level.startTime + zone.timeout * 1000 - level.time) / 1000));
				g_antirushWarned[n] = level.time + 2000;
			}
		}
	}
}

int Enh_CountryId(int clientNum)
{
	return (clientNum >= 0 && clientNum < MAX_CLIENTS) ? g_countryId[clientNum] : 0;
}

const char *Enh_CountryName(int clientNum)
{
	return EnhGeo_Name(Enh_CountryId(clientNum));
}

const char *Enh_CityName(int clientNum)
{
	if (clientNum < 0 || clientNum >= MAX_CLIENTS)
		return "";
	return g_cityName[clientNum];
}

const char *Enh_PlaceName(int clientNum)
{
	static char buf[128];
	const char *country = Enh_CountryName(clientNum);
	const char *city = Enh_CityName(clientNum);
	if (city && city[0]) {
		Com_sprintf(buf, sizeof(buf), "%s / %s", country, city);
		return buf;
	}
	return country;
}

static void bindLookup(int n, const char *ip)
{
	g_countryId[n] = EnhGeo_LookupIPv4(ip);
	Q_strncpyz(g_cityName[n], EnhGeo_LastCity(), sizeof(g_cityName[n]));
}

void Enh_BindCountry(gentity_t *ent, qboolean isBot)
{
	if (!ent || !ent->client)
		return;
	const int n = ent - g_entities;
	g_countryId[n] = 0;
	g_cityName[n][0] = 0;

	string behave = toLowerCopy(cvars::g_flagsbehaviour.svalue);
	if (isBot && !behave.empty()) {
		if (behave == "random") {
			g_countryId[n] = EnhGeo_RandomId();
			return;
		}
		bindLookup(n, behave.c_str());
		if (g_countryId[n])
			return;
	}

	char userinfo[MAX_INFO_STRING];
	trap_GetUserinfo(n, userinfo, sizeof(userinfo));
	bindLookup(n, Info_ValueForKey(userinfo, "ip"));
}

void Enh_ClientConnect(gentity_t *ent, qboolean firstTime, qboolean isBot)
{
	if (!ent || !ent->client || !firstTime)
		return;
	const int n = ent - g_entities;
	g_frozen[n] = qfalse;
	g_midget[n] = qfalse;
	g_killStreak[n] = 0;
	applyForceCvars(n);
	g_needConnectInfo[n] = (!isBot && g_common.advancedPlayerInfo) ? qtrue : qfalse;
}

void sendConnectInfo(int n)
{
	char userinfo[MAX_INFO_STRING];
	char ip[64];
	char ver[128];
	trap_GetUserinfo(n, userinfo, sizeof(userinfo));
	Q_strncpyz(ip, Info_ValueForKey(userinfo, "ip"), sizeof(ip));
	Q_strncpyz(ver, Info_ValueForKey(userinfo, "cl_etVersion"), sizeof(ver));
	if (!ver[0])
		Q_strncpyz(ver, Info_ValueForKey(userinfo, "cg_etVersion"), sizeof(ver));
	if (!ver[0])
		Q_strncpyz(ver, "unknown", sizeof(ver));
	const char *name = g_entities[n].client->pers.netname;
	const char *place = Enh_PlaceName(n);
	const string masked = maskIp(ip);
	const char *line = va("^5enhmod: ^7%s ^7connected with ip: %s (%s) client version: %s",
		name, masked.c_str(), place, ver);
	G_Printf("ENHMOD: %s connected with ip: %s (%s) client version: %s\n",
		name, masked.c_str(), place, ver);
	int viewers = 0;
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (!g_entities[i].inuse || !g_entities[i].client)
			continue;
		if (g_entities[i].client->pers.connected != CON_CONNECTED)
			continue;
		if (!Enh_HasFlag(i, 'I'))
			continue;
		trap_SendServerCommand(i, va("print \"%s\n\"", line));
		++viewers;
	}
	G_Printf("ENHMOD: connect info sent to %d client(s) with flag I\n", viewers);
}

void Enh_ClientBegin(gentity_t *ent)
{
	if (!ent || !ent->client)
		return;
	const int n = ent - g_entities;
	if (!g_needConnectInfo[n])
		return;
	g_needConnectInfo[n] = qfalse;
	if (!g_common.advancedPlayerInfo)
		return;
	sendConnectInfo(n);
}

void Enh_Obituary(gentity_t *self, gentity_t *attacker, int meansOfDeath)
{
	if (!self || !self->client)
		return;
	const int victim = self - g_entities;
	if (attacker && attacker->client && attacker != self) {
		const int killer = attacker - g_entities;
		g_lastKiller[victim] = killer;
		g_lastKilled[killer] = victim;
		if (g_common.multikillSeconds > 0 && !OnSameTeam(self, attacker)) {
			if (level.time - g_killStreakTime[killer] <= g_common.multikillSeconds * 1000)
				++g_killStreak[killer];
			else
				g_killStreak[killer] = 1;
			g_killStreakTime[killer] = level.time;
			if (g_killStreak[killer] >= 2) {
				trap_SendServerCommand(-1, va("cpm \"^5enhmod: ^7%s ^7multikill x%d\"",
					attacker->client->pers.netname, g_killStreak[killer]));
			}
		}
		if (!g_firstHeadshot && !OnSameTeam(self, attacker)
		    && (self->client->ps.eFlags & EF_HEADSHOT)) {
			g_firstHeadshot = qtrue;
			trap_SendServerCommand(-1, va("cpm \"^5enhmod: ^7%s ^7first headshot\"",
				attacker->client->pers.netname));
		}
	} else {
		g_killStreak[victim] = 0;
	}
}

void Enh_Damage(gentity_t *targ, gentity_t *attacker)
{
	if (!targ || !targ->client || !attacker || !attacker->client || targ == attacker)
		return;
	const int mode = cvars::g_drawAttackerHP.ivalue;
	if (!mode)
		return;
	const int hp = targ->health;
	if (mode & 1)
		trap_SendServerCommand(attacker - g_entities, va("print \"^5enhmod: ^7%s ^7HP %d\n\"", targ->client->pers.netname, hp));
	if (mode & 2)
		trap_SendServerCommand(attacker - g_entities, va("cpm \"^5enhmod: ^7%s ^7HP %d\"", targ->client->pers.netname, hp));
}

qboolean Enh_CallVoteAllowed(gentity_t *ent, const char *vote)
{
	(void)ent;
	if (!cvars::g_em_votemap.ivalue)
		return qtrue;
	if (!vote || !vote[0])
		return qfalse;
	return (!Q_stricmp(vote, "map") || !Q_stricmp(vote, "nextmap")
		|| !Q_stricmp(vote, "maprestart") || !Q_stricmp(vote, "campaign")) ? qtrue : qfalse;
}

bool Enh_CustomAllowed(int clientNum, const CustomCmd& c)
{
	return Enh_LevelAllowed(clientNum, c.levels) ? true : false;
}

const CustomCmd *Enh_FindCustom(const string& name)
{
	const string key = toLowerCopy(name);
	for (size_t i = 0; i < g_custom.size(); ++i) {
		if (g_custom[i].name == key)
			return &g_custom[i];
	}
	return 0;
}

static string playerCName(int clientNum)
{
	if (clientNum < 0 || clientNum >= MAX_CLIENTS || !g_entities[clientNum].client)
		return "";
	return g_entities[clientNum].client->pers.netname;
}

static string playerCleanName(int clientNum)
{
	char buf[MAX_NETNAME];
	Q_strncpyz(buf, playerCName(clientNum).c_str(), sizeof(buf));
	return Q_CleanStr(buf);
}

static void replaceAll(string& s, const string& a, const string& b)
{
	size_t p = 0;
	while ((p = s.find(a, p)) != string::npos) {
		s.replace(p, a.size(), b);
		p += b.size();
	}
}

static string expandTokens(int clientNum, string text)
{
	replaceAll(text, "<MYNAME_PLAYER_CNAME>", playerCName(clientNum));
	replaceAll(text, "<MYNAME_PLAYER_CSCNAME>", playerCleanName(clientNum));
	replaceAll(text, "<PLAYER_LAST_KILLER_CNAME>", playerCName(Enh_LastKiller(clientNum)));
	replaceAll(text, "<PLAYER_LAST_KILLED_CNAME>", playerCName(Enh_LastKilled(clientNum)));
	return text;
}

static void runExecPart(int clientNum, string part, const vector<string>& extra)
{
	part = trimCopy(expandTokens(clientNum, part));
	if (part.empty())
		return;
	if (part[0] == '!') {
		cmd::process((clientNum >= 0) ? &g_clientObjects[clientNum] : NULL, false, &part);
		return;
	}
	std::istringstream is(part);
	string verb;
	is >> verb;
	verb = toLowerCopy(verb);
	string rest;
	std::getline(is, rest);
	rest = trimCopy(rest);
	if (verb == "playsound") {
		trap_SendServerCommand(-1, va("playsound %s", rest.c_str()));
		return;
	}
	if (verb == "chat" || verb == "cchat") {
		trap_SendServerCommand(-1, va("chat \"%s\"", rest.c_str()));
		return;
	}
	if (verb == "cpm") {
		trap_SendServerCommand(-1, va("cpm \"%s\"", rest.c_str()));
		return;
	}
	string cmd = part;
	for (size_t i = 0; i < extra.size(); ++i) {
		cmd += " ";
		cmd += extra[i];
	}
	trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", cmd.c_str()));
}

qboolean Enh_TryCustom(int clientNum, const vector<string>& args)
{
	if (args.empty())
		return qfalse;
	const CustomCmd *c = Enh_FindCustom(args[0]);
	if (!c)
		return qfalse;
	if (!Enh_CustomAllowed(clientNum, *c)) {
		trap_SendServerCommand(clientNum, "cpm \"^5enhmod: ^7access denied\"");
		return qtrue;
	}
	vector<string> extra;
	for (size_t i = 1; i < args.size(); ++i)
		extra.push_back(args[i]);
	string exec = c->exec;
	size_t start = 0;
	for (;;) {
		size_t semi = exec.find(';', start);
		const string part = (semi == string::npos) ? exec.substr(start) : exec.substr(start, semi - start);
		runExecPart(clientNum, part, extra);
		if (semi == string::npos)
			break;
		start = semi + 1;
	}
	return qtrue;
}

qboolean Enh_AddAntiRushHere(gentity_t *ent, const char *key, int timeout, float radius, char method)
{
	if (!ent || !ent->client || !key || !key[0])
		return qfalse;
	AntiRushZone z;
	memset(&z, 0, sizeof(z));
	z.map = enhMapName();
	z.key = key;
	z.team = ent->client->sess.sessionTeam;
	VectorCopy(ent->r.currentOrigin, z.origin);
	z.radius = (radius > 0) ? radius : 300.f;
	z.timeout = (timeout > 0) ? timeout : 15;
	z.method = method ? method : 'r';
	g_zones.push_back(z);
	Enh_SaveAntiRush();
	return qtrue;
}

qboolean Enh_DelAntiRush(const char *key)
{
	if (!key || !key[0])
		return qfalse;
	return Enh_DelZone(enhMapName(), key);
}

void Enh_ListAntiRush(gentity_t *ent)
{
	const string map = enhMapName();
	int n = 0;
	for (size_t i = 0; i < g_zones.size(); ++i) {
		if (g_zones[i].map != map)
			continue;
		++n;
		const char *line = va("^5enhmod: ^7%s team %d r=%.0f t=%d %c",
			g_zones[i].key.c_str(), g_zones[i].team, g_zones[i].radius,
			g_zones[i].timeout, g_zones[i].method);
		if (ent)
			trap_SendServerCommand(ent - g_entities, va("print \"%s\n\"", line));
		else
			G_Printf("%s\n", line);
	}
	if (!n) {
		if (ent)
			trap_SendServerCommand(ent - g_entities, "print \"^5enhmod: ^7no antirush zones on this map\n\"");
		else
			G_Printf("ENHMOD: no antirush zones on this map\n");
	}
}
