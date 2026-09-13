#ifndef BGAME_ENGINE_H
#define BGAME_ENGINE_H

#include <stddef.h>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////

/* Static class for global ET engine functions.
 *
 * This class should only contain functions common to both game/cgame.
 * This class is static and cgame/game must implement Engine.cpp.
 *
 * Ptr is intptr_t on all hosts so 64-bit etlded can pass pointers through
 * the syscall varargs without truncating them.
 */
class Engine
{
public:
    typedef intptr_t (QDECL* Ptr)( intptr_t arg, ... );

    static Ptr ptr;

    static size_t argc ( );
    static size_t argl ( string& );
    static size_t args ( vector<string>& );
    static size_t args ( vector<string>&, const string& );
    static size_t argv ( size_t, string& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_ENGINE_H
