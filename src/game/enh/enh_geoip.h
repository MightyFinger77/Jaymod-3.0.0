#ifndef GAME_ENH_GEOIP_H
#define GAME_ENH_GEOIP_H

// MaxMind lookup. Prefers GeoLite2-City.mmdb (country + city), then
// GeoLite2-Country.mmdb, then legacy GeoIP.dat. No database is shipped.

void EnhGeo_Init();
void EnhGeo_Shutdown();
int EnhGeo_LookupIPv4(const char *ip); // 0 = unknown; city in EnhGeo_LastCity()
const char *EnhGeo_LastCity();
const char *EnhGeo_Code(int id);
const char *EnhGeo_Name(int id);
int EnhGeo_RandomId();

#endif
