#include <bgame/impl.h>
#include <game/enh/enh_geoip.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

const int COUNTRY_BEGIN = 16776960;
const int MAX_GEO_ID = 256;

const char *const kCode[MAX_GEO_ID] = {
	"--","AP","EU","AD","AE","AF","AG","AI","AL","AM","CW",
	"AO","AQ","AR","AS","AT","AU","AW","AZ","BA","BB",
	"BD","BE","BF","BG","BH","BI","BJ","BM","BN","BO",
	"BR","BS","BT","BV","BW","BY","BZ","CA","CC","CD",
	"CF","CG","CH","CI","CK","CL","CM","CN","CO","CR",
	"CU","CV","CX","CY","CZ","DE","DJ","DK","DM","DO",
	"DZ","EC","EE","EG","EH","ER","ES","ET","FI","FJ",
	"FK","FM","FO","FR","FX","GA","GB","GD","GE","GF",
	"GH","GI","GL","GM","GN","GP","GQ","GR","GS","GT",
	"GU","GW","GY","HK","HM","HN","HR","HT","HU","ID",
	"IE","IL","IN","IO","IQ","IR","IS","IT","JM","JO",
	"JP","KE","KG","KH","KI","KM","KN","KP","KR","KW",
	"KY","KZ","LA","LB","LC","LI","LK","LR","LS","LT",
	"LU","LV","LY","MA","MC","MD","MG","MH","MK","ML",
	"MM","MN","MO","MP","MQ","MR","MS","MT","MU","MV",
	"MW","MX","MY","MZ","NA","NC","NE","NF","NG","NI",
	"NL","NO","NP","NR","NU","NZ","OM","PA","PE","PF",
	"PG","PH","PK","PL","PM","PN","PR","PS","PT","PW",
	"PY","QA","RE","RO","RU","RW","SA","SB","SC","SD",
	"SE","SG","SH","SI","SJ","SK","SL","SM","SN","SO",
	"SR","ST","SV","SY","SZ","TC","TD","TF","TG","TH",
	"TJ","TK","TM","TN","TO","TL","TR","TT","TV","TW",
	"TZ","UA","UG","UM","US","UY","UZ","VA","VC","VE",
	"VG","VI","VN","VU","WF","WS","YE","YT","RS","ZA",
	"ZM","ME","ZW","A1","A2","O1","AX","GG","IM","JE",
	"BL","MF","BQ","SS","O1"
};

const char *const kName[MAX_GEO_ID] = {
	"unknown","Asia/Pacific Region","Europe","Andorra","United Arab Emirates","Afghanistan",
	"Antigua and Barbuda","Anguilla","Albania","Armenia","Curacao","Angola","Antarctica",
	"Argentina","American Samoa","Austria","Australia","Aruba","Azerbaijan","Bosnia and Herzegovina",
	"Barbados","Bangladesh","Belgium","Burkina Faso","Bulgaria","Bahrain","Burundi","Benin",
	"Bermuda","Brunei","Bolivia","Brazil","Bahamas","Bhutan","Bouvet Island","Botswana","Belarus",
	"Belize","Canada","Cocos Islands","Congo (DRC)","Central African Republic","Congo","Switzerland",
	"Cote D'Ivoire","Cook Islands","Chile","Cameroon","China","Colombia","Costa Rica","Cuba",
	"Cape Verde","Christmas Island","Cyprus","Czech Republic","Germany","Djibouti","Denmark",
	"Dominica","Dominican Republic","Algeria","Ecuador","Estonia","Egypt","Western Sahara","Eritrea",
	"Spain","Ethiopia","Finland","Fiji","Falkland Islands","Micronesia","Faroe Islands","France",
	"France (Metropolitan)","Gabon","United Kingdom","Grenada","Georgia","French Guiana","Ghana",
	"Gibraltar","Greenland","Gambia","Guinea","Guadeloupe","Equatorial Guinea","Greece",
	"South Georgia","Guatemala","Guam","Guinea-Bissau","Guyana","Hong Kong","Heard Island","Honduras",
	"Croatia","Haiti","Hungary","Indonesia","Ireland","Israel","India","British Indian Ocean Territory",
	"Iraq","Iran","Iceland","Italy","Jamaica","Jordan","Japan","Kenya","Kyrgyzstan","Cambodia",
	"Kiribati","Comoros","Saint Kitts and Nevis","North Korea","South Korea","Kuwait","Cayman Islands",
	"Kazakhstan","Laos","Lebanon","Saint Lucia","Liechtenstein","Sri Lanka","Liberia","Lesotho",
	"Lithuania","Luxembourg","Latvia","Libya","Morocco","Monaco","Moldova","Madagascar","Marshall Islands",
	"Macedonia","Mali","Myanmar","Mongolia","Macau","Northern Mariana Islands","Martinique","Mauritania",
	"Montserrat","Malta","Mauritius","Maldives","Malawi","Mexico","Malaysia","Mozambique","Namibia",
	"New Caledonia","Niger","Norfolk Island","Nigeria","Nicaragua","Netherlands","Norway","Nepal",
	"Nauru","Niue","New Zealand","Oman","Panama","Peru","French Polynesia","Papua New Guinea",
	"Philippines","Pakistan","Poland","Saint Pierre and Miquelon","Pitcairn Islands","Puerto Rico",
	"Palestine","Portugal","Palau","Paraguay","Qatar","Reunion","Romania","Russia","Rwanda",
	"Saudi Arabia","Solomon Islands","Seychelles","Sudan","Sweden","Singapore","Saint Helena",
	"Slovenia","Svalbard and Jan Mayen","Slovakia","Sierra Leone","San Marino","Senegal","Somalia",
	"Suriname","Sao Tome and Principe","El Salvador","Syria","Swaziland","Turks and Caicos Islands",
	"Chad","French Southern Territories","Togo","Thailand","Tajikistan","Tokelau","Turkmenistan",
	"Tunisia","Tonga","Timor-Leste","Turkey","Trinidad and Tobago","Tuvalu","Taiwan","Tanzania",
	"Ukraine","Uganda","US Minor Outlying Islands","United States","Uruguay","Uzbekistan","Vatican",
	"Saint Vincent","Venezuela","British Virgin Islands","US Virgin Islands","Vietnam","Vanuatu",
	"Wallis and Futuna","Samoa","Yemen","Mayotte","Serbia","South Africa","Zambia","Montenegro",
	"Zimbabwe","Anonymous Proxy","Satellite Provider","Other","Aland Islands","Guernsey","Isle of Man",
	"Jersey","Saint Barthelemy","Saint Martin","Bonaire","South Sudan","Other"
};

const unsigned char kMetaMark[] = {
	0xab, 0xcd, 0xef, 'M', 'a', 'x', 'M', 'i', 'n', 'd', '.', 'c', 'o', 'm'
};

unsigned char *g_data = 0;
int g_size = 0;
int g_begin = COUNTRY_BEGIN;
qboolean g_dat = qfalse;
qboolean g_mmdb = qfalse;
char g_lookupCity[64];
unsigned int g_nodes = 0;
unsigned int g_recordBits = 0;
unsigned int g_ipVersion = 0;
unsigned int g_treeBytes = 0;
unsigned int g_ipv4Root = 0;

int idFromIso(const char *iso)
{
	if (!iso || !iso[0])
		return 0;
	char a = (char)toupper((unsigned char)iso[0]);
	char b = (char)toupper((unsigned char)iso[1]);
	for (int i = 1; i < MAX_GEO_ID; ++i) {
		if (kCode[i][0] == a && kCode[i][1] == b)
			return i;
	}
	return 0;
}

unsigned long ipv4ToNum(const char *addr)
{
	if (!addr || !addr[0])
		return 0;
	unsigned a = 0, b = 0, c = 0, d = 0;
	if (sscanf(addr, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
		return 0;
	if (a > 255 || b > 255 || c > 255 || d > 255)
		return 0;
	return (a << 24) | (b << 16) | (c << 8) | d;
}

void stripPort(char *ip)
{
	char *colon = strchr(ip, ':');
	if (colon)
		*colon = '\0';
}

qboolean privateIPv4(unsigned long n)
{
	if ((n >> 24) == 10 || (n >> 24) == 127)
		return qtrue;
	if ((n >> 24) == 192 && ((n >> 16) & 255) == 168)
		return qtrue;
	if ((n >> 24) == 172 && ((n >> 16) & 255) >= 16 && ((n >> 16) & 255) <= 31)
		return qtrue;
	return qfalse;
}

unsigned int be24(const unsigned char *p)
{
	return ((unsigned int)p[0] << 16) | ((unsigned int)p[1] << 8) | p[2];
}

unsigned int be32(const unsigned char *p)
{
	return ((unsigned int)p[0] << 24) | ((unsigned int)p[1] << 16)
		| ((unsigned int)p[2] << 8) | p[3];
}

struct Cursor {
	const unsigned char *base;
	const unsigned char *end;
	const unsigned char *p;
};

qboolean curOk(const Cursor& c, unsigned int n)
{
	return (c.p && c.end && c.p + n <= c.end) ? qtrue : qfalse;
}

unsigned int readBe(Cursor& c, int n)
{
	unsigned int v = 0;
	if (!curOk(c, (unsigned int)n))
		return 0;
	for (int i = 0; i < n; ++i)
		v = (v << 8) | *c.p++;
	return v;
}

qboolean readCtrl(Cursor& c, int& type, unsigned int& size)
{
	if (!curOk(c, 1))
		return qfalse;
	const unsigned char b = *c.p++;
	type = b >> 5;
	size = b & 31;
	if (type == 0) {
		if (!curOk(c, 1))
			return qfalse;
		type = *c.p++ + 7;
	}
	if (type == 1) {
		const int ss = (int)((b >> 3) & 3);
		const unsigned int vvv = b & 7;
		if (ss == 0)
			size = (vvv << 8) | readBe(c, 1);
		else if (ss == 1)
			size = ((vvv << 16) | readBe(c, 2)) + 2048;
		else if (ss == 2)
			size = ((vvv << 24) | readBe(c, 3)) + 526336;
		else
			size = readBe(c, 4);
		return qtrue;
	}
	if (size >= 29) {
		if (size == 29)
			size = 29 + readBe(c, 1);
		else if (size == 30)
			size = 285 + readBe(c, 2);
		else
			size = 65821 + readBe(c, 3);
	}
	return qtrue;
}

qboolean skipValue(Cursor& c);

qboolean decodePtr(Cursor& c, Cursor& out)
{
	int type = 0;
	unsigned int size = 0;
	if (!readCtrl(c, type, size) || type != 1)
		return qfalse;
	out.base = c.base;
	out.end = c.end;
	out.p = c.base + size;
	return (out.p < out.end) ? qtrue : qfalse;
}

qboolean peekType(const Cursor& c, int& type)
{
	Cursor peek = c;
	unsigned int size = 0;
	return readCtrl(peek, type, size);
}

qboolean skipValue(Cursor& c)
{
	int type = 0;
	if (!peekType(c, type))
		return qfalse;
	if (type == 1) {
		Cursor tgt;
		if (!decodePtr(c, tgt))
			return qfalse;
		return skipValue(tgt);
	}
	unsigned int size = 0;
	if (!readCtrl(c, type, size))
		return qfalse;
	if (type == 7) {
		for (unsigned int i = 0; i < size; ++i) {
			if (!skipValue(c) || !skipValue(c))
				return qfalse;
		}
		return qtrue;
	}
	if (type == 11) {
		for (unsigned int i = 0; i < size; ++i) {
			if (!skipValue(c))
				return qfalse;
		}
		return qtrue;
	}
	if (type == 3)
		size = 8;
	if (type == 15)
		size = 4;
	if (type == 14)
		return qtrue;
	if (!curOk(c, size))
		return qfalse;
	c.p += size;
	return qtrue;
}

qboolean readUtf8(Cursor& c, char *out, int outSize)
{
	int type = 0;
	if (!peekType(c, type))
		return qfalse;
	if (type == 1) {
		Cursor tgt;
		if (!decodePtr(c, tgt))
			return qfalse;
		return readUtf8(tgt, out, outSize);
	}
	unsigned int size = 0;
	if (!readCtrl(c, type, size) || type != 2)
		return qfalse;
	if (!curOk(c, size))
		return qfalse;
	if (outSize < 2)
		return qfalse;
	const int n = (size >= (unsigned int)(outSize - 1)) ? (outSize - 1) : (int)size;
	memcpy(out, c.p, (size_t)n);
	out[n] = 0;
	c.p += size;
	return qtrue;
}

unsigned int readUint(Cursor& c)
{
	int type = 0;
	if (!peekType(c, type))
		return 0;
	if (type == 1) {
		Cursor tgt;
		if (!decodePtr(c, tgt))
			return 0;
		return readUint(tgt);
	}
	unsigned int size = 0;
	if (!readCtrl(c, type, size))
		return 0;
	if (type == 5 || type == 6 || type == 8 || type == 9)
		return readBe(c, (int)size);
	skipValue(c);
	return 0;
}

qboolean mapGet(Cursor c, const char *key, Cursor& value)
{
	int type = 0;
	if (!peekType(c, type))
		return qfalse;
	if (type == 1) {
		Cursor tgt;
		if (!decodePtr(c, tgt))
			return qfalse;
		return mapGet(tgt, key, value);
	}
	unsigned int pairs = 0;
	if (!readCtrl(c, type, pairs) || type != 7)
		return qfalse;
	for (unsigned int i = 0; i < pairs; ++i) {
		char k[64];
		if (!readUtf8(c, k, sizeof(k)))
			return qfalse;
		if (!Q_stricmp(k, key)) {
			value = c;
			return qtrue;
		}
		if (!skipValue(c))
			return qfalse;
	}
	return qfalse;
}

int parseMeta(const unsigned char *file, int len)
{
	int mark = -1;
	const int mlen = (int)sizeof(kMetaMark);
	for (int i = 0; i + mlen <= len; ++i) {
		if (memcmp(file + i, kMetaMark, (size_t)mlen) == 0)
			mark = i;
	}
	if (mark < 0 || mark + mlen >= len)
		return 0;
	Cursor c;
	c.base = file + mark + mlen;
	c.end = file + len;
	c.p = c.base;
	Cursor v;
	if (!mapGet(c, "node_count", v))
		return 0;
	g_nodes = readUint(v);
	if (!mapGet(c, "record_size", v))
		return 0;
	g_recordBits = readUint(v);
	if (!mapGet(c, "ip_version", v))
		return 0;
	g_ipVersion = readUint(v);
	if (!g_nodes || (g_recordBits != 24 && g_recordBits != 28 && g_recordBits != 32))
		return 0;
	g_treeBytes = (g_recordBits * 2 / 8) * g_nodes;
	if (g_treeBytes + 16 >= (unsigned int)len)
		return 0;
	return 1;
}

qboolean readNode(unsigned int node, int bit, unsigned int& rec)
{
	const unsigned int bpn = g_recordBits * 2 / 8;
	const unsigned int pos = node * bpn;
	if (pos + bpn > (unsigned int)g_size)
		return qfalse;
	const unsigned char *p = g_data + pos;
	unsigned int left = 0, right = 0;
	if (g_recordBits == 24) {
		left = be24(p);
		right = be24(p + 3);
	} else if (g_recordBits == 28) {
		left = ((unsigned int)(p[3] >> 4) << 24) | be24(p);
		right = ((unsigned int)(p[3] & 0x0f) << 24) | be24(p + 4);
	} else {
		left = be32(p);
		right = be32(p + 4);
	}
	rec = bit ? right : left;
	return qtrue;
}

int walkBits(unsigned int start, unsigned long value, int bits)
{
	unsigned int node = start;
	for (int i = bits - 1; i >= 0; --i) {
		unsigned int rec = 0;
		if (!readNode(node, (int)((value >> i) & 1), rec))
			return 0;
		if (rec == g_nodes)
			return 0;
		if (rec > g_nodes) {
			const unsigned int fileOff = (rec - g_nodes) + g_treeBytes;
			if (fileOff >= (unsigned int)g_size)
				return 0;
			Cursor recCur;
			recCur.base = g_data + g_treeBytes + 16;
			recCur.end = g_data + g_size;
			recCur.p = g_data + fileOff;
			Cursor cityMap, names, en;
			if (mapGet(recCur, "city", cityMap)
			    && mapGet(cityMap, "names", names)
			    && mapGet(names, "en", en))
				readUtf8(en, g_lookupCity, sizeof(g_lookupCity));
			Cursor country, iso;
			if (!mapGet(recCur, "country", country)
			    && !mapGet(recCur, "registered_country", country))
				return 0;
			if (!mapGet(country, "iso_code", iso))
				return 0;
			char code[8];
			if (!readUtf8(iso, code, sizeof(code)))
				return 0;
			return idFromIso(code);
		}
		node = rec;
	}
	return 0;
}

int lookupMmdb(unsigned long ip)
{
	if (g_ipVersion == 4)
		return walkBits(0, ip, 32);
	unsigned int node = 0;
	for (int i = 0; i < 96; ++i) {
		unsigned int rec = 0;
		if (!readNode(node, 0, rec))
			return 0;
		if (rec >= g_nodes)
			break;
		node = rec;
	}
	g_ipv4Root = node;
	return walkBits(node, ip, 32);
}

int lookupDat(unsigned long n)
{
	unsigned int offset = 0;
	for (int depth = 31; depth >= 0; --depth) {
		const unsigned int pos = offset * 6;
		if (pos + 6 > (unsigned int)g_size)
			return 0;
		const unsigned char *p = g_data + pos;
		const unsigned int left = p[0] | (p[1] << 8) | (p[2] << 16);
		const unsigned int right = p[3] | (p[4] << 8) | (p[5] << 16);
		const unsigned int next = (n & (1u << depth)) ? right : left;
		if (next >= (unsigned int)g_begin) {
			const int id = (int)(next - (unsigned int)g_begin);
			if (id <= 0 || id >= MAX_GEO_ID)
				return 0;
			return id;
		}
		offset = next;
	}
	return 0;
}

qboolean loadBytes(const char *name)
{
	fileHandle_t f;
	const int len = trap_FS_FOpenFile(name, &f, FS_READ);
	if (len <= 0 || !f)
		return qfalse;
	unsigned char *buf = (unsigned char *)malloc((size_t)len);
	if (!buf) {
		trap_FS_FCloseFile(f);
		return qfalse;
	}
	trap_FS_Read(buf, len, f);
	trap_FS_FCloseFile(f);
	if (g_data)
		free(g_data);
	g_data = buf;
	g_size = len;
	return qtrue;
}

} // namespace

void EnhGeo_Init()
{
	EnhGeo_Shutdown();
	static const char *const mmdbNames[] = {
		"GeoLite2-City.mmdb",
		"geoip/GeoLite2-City.mmdb",
		"GeoIP2-City.mmdb",
		"GeoLite2-Country.mmdb",
		"geoip/GeoLite2-Country.mmdb",
		"GeoIP2-Country.mmdb",
		0
	};
	for (int i = 0; mmdbNames[i]; ++i) {
		if (!loadBytes(mmdbNames[i]))
			continue;
		if (parseMeta(g_data, g_size)) {
			g_mmdb = qtrue;
			G_Printf("ENHMOD: %s loaded (%d bytes, %u nodes)\n", mmdbNames[i], g_size, g_nodes);
			return;
		}
		G_Printf("ENHMOD: %s is not a valid MaxMind DB\n", mmdbNames[i]);
	}
	if (loadBytes("GeoIP.dat")) {
		g_dat = qtrue;
		g_begin = COUNTRY_BEGIN;
		G_Printf("ENHMOD: GeoIP.dat loaded (%d bytes)\n", g_size);
		return;
	}
	G_Printf("ENHMOD: no GeoLite2-City.mmdb, GeoLite2-Country.mmdb, or GeoIP.dat (country unknown)\n");
}

void EnhGeo_Shutdown()
{
	if (g_data)
		free(g_data);
	g_data = 0;
	g_size = 0;
	g_dat = qfalse;
	g_mmdb = qfalse;
	g_nodes = 0;
	g_recordBits = 0;
	g_ipVersion = 0;
	g_treeBytes = 0;
	g_ipv4Root = 0;
	g_lookupCity[0] = 0;
}

int EnhGeo_LookupIPv4(const char *ipRaw)
{
	g_lookupCity[0] = 0;
	if (!g_data || !ipRaw)
		return 0;
	char ip[64];
	Q_strncpyz(ip, ipRaw, sizeof(ip));
	stripPort(ip);
	const unsigned long n = ipv4ToNum(ip);
	if (!n || privateIPv4(n))
		return 0;
	if (g_mmdb)
		return lookupMmdb(n);
	if (g_dat)
		return lookupDat(n);
	return 0;
}

const char *EnhGeo_LastCity()
{
	return g_lookupCity;
}

const char *EnhGeo_Code(int id)
{
	if (id <= 0 || id >= MAX_GEO_ID)
		return "--";
	return kCode[id];
}

const char *EnhGeo_Name(int id)
{
	if (id <= 0 || id >= MAX_GEO_ID)
		return "unknown";
	return kName[id];
}

int EnhGeo_RandomId()
{
	int id = 1 + (rand() % (MAX_GEO_ID - 2));
	if (id <= 0 || id >= MAX_GEO_ID)
		id = 56;
	return id;
}
