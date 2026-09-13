#include <bgame/impl.h>
#include <omnibot/common/BotExports.h>
#include <windows.h>

extern bool   g_IsOmnibotLoaded;
extern string g_OmnibotLibPath;

///////////////////////////////////////////////////////////////////////////////

namespace {
    const char* const __LIB_SUFFIX = ".dll";
    const char* const __OMNI_DIR   = "omni-bot";
    const char* const __OMNI_LOG   = "OMNIBOT:";
    const char        __PATHSEP    = '\\';

    set<string>  __dirSet;
    list<string> __dirList;
    HINSTANCE    __handle = NULL;

    void addDirectory( string dir ) {
        // skip if already present
        if (!__dirSet.insert( dir ).second)
            return;

        __dirList.push_back( dir );
    }
}

///////////////////////////////////////////////////////////////////////////////

static const char*
win32LoadError()
{
    static char buf[256];
    const DWORD code = GetLastError();
    buf[0] = '\0';
    FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, code, 0, buf, sizeof(buf), NULL );
    for ( char* p = buf; *p; ++p ) {
        if (*p == '\r' || *p == '\n')
            *p = ' ';
    }
    return va( "%lu %s", (unsigned long)code, buf );
}

///////////////////////////////////////////////////////////////////////////////

eomnibot_error
Omnibot_LoadLibrary( const int version, const char* const libbase, const char* const customdir )
{
    G_Printf( "%s loader version 0.81\n", __OMNI_LOG );

    __dirSet.clear();
    __dirList.clear();

    // ETL Win64 ships omnibot_et_x64.dll. omnibot_et.dll is 32-bit and cannot
    // be loaded by a 64-bit qagame (LoadLibrary error 193).
    vector<string> libnames;
#if defined(_WIN64) || defined(__x86_64__) || defined(__amd64__)
    libnames.push_back( string(libbase) + "_x64" + __LIB_SUFFIX );
#endif
    libnames.push_back( string(libbase) + __LIB_SUFFIX );

    // prepare list of path names
    set<string>   dirSet;
    list<string*> dirList;

    // add custom dir
    if (customdir && customdir[0])
        addDirectory( customdir );

    // add fs_homepath based dir
    {
        char buffer[1024];
        trap_Cvar_VariableStringBuffer( "fs_homepath", buffer, sizeof(buffer) );
        if (*buffer) {
            ostringstream path;
            path << buffer << __PATHSEP << __OMNI_DIR;
            addDirectory( path.str() );
        }
    }

    // add fs_basepath based dir
    {
        char buffer[1024];
        trap_Cvar_VariableStringBuffer( "fs_basepath", buffer, sizeof(buffer) );
        if (*buffer) {
            ostringstream path;
            path << buffer << __PATHSEP << __OMNI_DIR;
            addDirectory( path.str() );
        }
    }

    // add "ProgramFiles" based dir
    {
        const char* pf = getenv( "ProgramFiles" );
        if (pf && *pf) {
            ostringstream path;
            path << pf << __PATHSEP << __OMNI_DIR;
            addDirectory( path.str() );
        }
    }

    // add special zero-length string for final try
    addDirectory( "" );

    // print search path
    G_Printf( "%s search path:\n", __OMNI_LOG );

    const list<string>::iterator end = __dirList.end();
    for ( list<string>::iterator it = __dirList.begin(); it != end; it++ ) {
        const string& dir = *it;

        if (dir.length())
            G_Printf( "    %s\n", dir.c_str() );
        else
            G_Printf( "    <SYSTEM-LOADER>\n" );
    }

    // attempt loading
    for ( list<string>::iterator it = __dirList.begin(); it != end; it++ ) {
        const string& dir = *it;

        string name = dir;
        if (name.length() && name[ name.length()-1 ] != __PATHSEP)
            name += __PATHSEP;

        for ( size_t li = 0; li < libnames.size(); ++li ) {
            string tryname = name + libnames[li];
            const bool absPath = (tryname.length() >= 2 && tryname[1] == ':')
                || (tryname.length() >= 2 && tryname[0] == '\\');
            if (absPath)
                __handle = LoadLibraryExA( tryname.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
            else
                __handle = LoadLibraryA( tryname.c_str() );

            if (__handle) {
                G_Printf( "%s load '%s': success\n", __OMNI_LOG, tryname.c_str() );
                name = tryname;
                break;
            }

            G_Printf( "%s load '%s': failure (%s)\n",
                __OMNI_LOG, tryname.c_str(), win32LoadError() );
        }

        if (!__handle)
            continue;

        // success, update buffer indicating bot path
        g_OmnibotLibPath = name;
        break;
    }

    if (!__handle)
        return BOT_ERROR_CANTLOADDLL;

    // check proper dll and initialize it
    pfnGetFunctionsFromDLL pfnGetBotFuncs = 0;
    memset( &g_BotFunctions, 0, sizeof(g_BotFunctions) );

    pfnGetBotFuncs = (pfnGetFunctionsFromDLL)GetProcAddress( __handle, "ExportBotFunctionsFromDLL" );
    G_Printf( "%s address-lookup: %s\n", __OMNI_LOG, pfnGetBotFuncs ? "success" : "failure" );
    if (!pfnGetBotFuncs) {
        G_Printf( "%s GetProcAddress failed\n", __OMNI_LOG );
        return BOT_ERROR_CANTGETBOTFUNCTIONS;
    }

    eomnibot_error oe = pfnGetBotFuncs( &g_BotFunctions, sizeof(g_BotFunctions) );
    G_Printf( "%s pfnGetBotFuncs: %s\n", __OMNI_LOG, oe == BOT_ERROR_NONE ? "success" : "failure" );
	if (oe != BOT_ERROR_NONE) {
		G_Printf("%s pfnGetBotFuncs failed - %s\n", __OMNI_LOG, Omnibot_ErrorString(oe));
		Omnibot_FreeLibrary();
        return oe;
	}

    oe = g_BotFunctions.pfnInitialize( Bot_GetBotVisibleInterface(), version );
    g_IsOmnibotLoaded = (oe == BOT_ERROR_NONE);

    G_Printf( "%s initialization: %s\n", __OMNI_LOG, g_IsOmnibotLoaded ? "success" : "failure" );
    return oe;
}

///////////////////////////////////////////////////////////////////////////////

void
Omnibot_FreeLibrary()
{
	if (__handle) {
	    FreeLibrary( __handle );
		__handle = NULL;
	}

    memset( &g_BotFunctions, 0, sizeof(g_BotFunctions) );

	delete g_InterfaceFunctions;
	g_InterfaceFunctions = 0;

    g_IsOmnibotLoaded = false;
}
