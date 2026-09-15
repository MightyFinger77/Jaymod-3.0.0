cmake_minimum_required(VERSION 3.16)
project(jaymod LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_C_STANDARD 99)

string(TIMESTAMP JAYMOD_BUILD_DATE "%a %b %d %H:%M:%S UTC %Y" UTC)

if(WIN32)
    set(JAYMOD_PLATFORM_NAME "Windows")
    set(JAYMOD_PLATFORM_NAMEF "windows")
    set(JAYMOD_PLATFORM_DEFINE "WINDOWS")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(JAYMOD_BUILD_TARGET "windows-x64")
        set(QAGAME_OUTPUT_NAME "qagame_mp_x64")
        set(CGAME_OUTPUT_NAME "cgame_mp_x64")
        set(UI_OUTPUT_NAME "ui_mp_x64")
        set(JAYMOD_ALT_ARCH "x86_64")
    else()
        set(JAYMOD_BUILD_TARGET "windows-x86")
        set(QAGAME_OUTPUT_NAME "qagame_mp_x86")
        set(CGAME_OUTPUT_NAME "cgame_mp_x86")
        set(UI_OUTPUT_NAME "ui_mp_x86")
        set(JAYMOD_ALT_ARCH "")
    endif()
else()
    set(JAYMOD_PLATFORM_NAME "Linux")
    set(JAYMOD_PLATFORM_NAMEF "linux")
    set(JAYMOD_PLATFORM_DEFINE "LINUX")
    set(JAYMOD_BUILD_TARGET "linux-x64")
    set(QAGAME_OUTPUT_NAME "qagame.mp.x86_64")
    set(CGAME_OUTPUT_NAME "cgame.mp.x86_64")
    set(UI_OUTPUT_NAME "ui.mp.x86_64")
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/base")
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/project.h.in"
    "${CMAKE_BINARY_DIR}/generated/base/project.h"
    @ONLY
)

set(LUA_SRCS
    src/lua/lapi.c
    src/lua/lcode.c
    src/lua/ldebug.c
    src/lua/ldo.c
    src/lua/ldump.c
    src/lua/lfunc.c
    src/lua/lgc.c
    src/lua/llex.c
    src/lua/lmem.c
    src/lua/lobject.c
    src/lua/lopcodes.c
    src/lua/lparser.c
    src/lua/lstate.c
    src/lua/lstring.c
    src/lua/ltable.c
    src/lua/ltm.c
    src/lua/lundump.c
    src/lua/lvm.c
    src/lua/lzio.c
    src/lua/lauxlib.c
    src/lua/lbaselib.c
    src/lua/ldblib.c
    src/lua/liolib.c
    src/lua/lmathlib.c
    src/lua/loslib.c
    src/lua/ltablib.c
    src/lua/lstrlib.c
    src/lua/loadlib.c
    src/lua/linit.c
)
set_source_files_properties(${LUA_SRCS} PROPERTIES LANGUAGE C)

file(GLOB GAME_SRCS CONFIGURE_DEPENDS
    src/game/*.cpp
    src/game/cmd/*.cpp
    src/game/cvar/*.cpp
    src/game/str/*.cpp
    src/game/lua/*.cpp
    src/game/enh/*.cpp
)

file(GLOB BGAME_SRCS
    src/bgame/*.cpp
    src/bgame/cvar/*.cpp
    src/bgame/str/*.cpp
)

file(GLOB BASE_SRCS
    src/base/*.cpp
    src/base/base64/*.cpp
    src/base/str/*.cpp
    src/base/text/*.cpp
    src/base/lua/*.cpp
)

file(GLOB OMNIBOT_SRCS
    src/omnibot/common/*.cpp
    src/omnibot/et/*.cpp
)

if(WIN32)
    file(GLOB BGAME_PLATFORM_SRCS src/bgame/win32/*.cpp)
    file(GLOB OMNIBOT_PLATFORM_SRCS src/omnibot/win32/*.cpp)
    file(GLOB CGAME_PLATFORM_SRCS src/cgame/win32/*.cpp)
else()
    file(GLOB BGAME_PLATFORM_SRCS src/bgame/linux/*.cpp)
    file(GLOB OMNIBOT_PLATFORM_SRCS src/omnibot/linux/*.cpp)
    file(GLOB CGAME_PLATFORM_SRCS src/cgame/linux/*.cpp)
endif()

file(GLOB CGAME_SRCS
    src/cgame/*.cpp
    src/cgame/cvar/*.cpp
)

set(CGAME_BGAME_SRCS
    src/bgame/Engine.cpp
    src/bgame/Process.cpp
    src/bgame/SampledStat.cpp
    src/bgame/bg_animation.cpp
    src/bgame/bg_animgroup.cpp
    src/bgame/bg_attack2.cpp
    src/bgame/bg_character.cpp
    src/bgame/bg_classes.cpp
    src/bgame/bg_jaymod.cpp
    src/bgame/bg_misc.cpp
    src/bgame/bg_molotov.cpp
    src/bgame/bg_pmove.cpp
    src/bgame/bg_slidemove.cpp
    src/bgame/bg_sscript.cpp
    src/bgame/bg_stats.cpp
    src/bgame/bg_tracemap.cpp
    src/bgame/q_math.cpp
    src/bgame/q_shared.cpp
    src/bgame/cvar/Cvar.cpp
    src/bgame/str/util.cpp
    src/bgame/str/PairDump.cpp
)

file(GLOB UI_SRCS
    src/ui/*.cpp
    src/ui/cvar/*.cpp
)

set(UI_BGAME_SRCS
    src/bgame/Process.cpp
    src/bgame/bg_campaign.cpp
    src/bgame/bg_classes.cpp
    src/bgame/bg_jaymod.cpp
    src/bgame/bg_misc.cpp
    src/bgame/q_math.cpp
    src/bgame/q_shared.cpp
    src/bgame/cvar/Cvar.cpp
)

function(jaymod_configure_module target extra_defs extra_libs)
    set_target_properties(${target} PROPERTIES
        PREFIX ""
        CXX_STANDARD 98
        C_STANDARD 99
    )

    target_include_directories(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/src/lua"
        "${CMAKE_SOURCE_DIR}/pak"
        "${CMAKE_BINARY_DIR}/generated"
    )

    target_compile_definitions(${target} PRIVATE
        ${extra_defs}
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_WARNINGS
        NOMINMAX
    )

    if(WIN32)
        target_link_libraries(${target} PRIVATE ${extra_libs})
        if(MSVC)
            target_compile_options(${target} PRIVATE
                /EHsc /GS- /W1 /wd4996 /wd4244 /wd4305 /wd4800 /wd4355 /MP /bigobj
            )
            target_link_options(${target} PRIVATE /INCREMENTAL:NO)
        else()
            target_compile_options(${target} PRIVATE
                -fno-strict-aliasing -fpermissive -Wno-unused-parameter -Wno-write-strings
            )
            target_link_options(${target} PRIVATE -static-libgcc -static-libstdc++ -static)
        endif()
    else()
        target_compile_definitions(${target} PRIVATE JAYMOD_LINUX)
        target_link_libraries(${target} PRIVATE m dl ${extra_libs})
        target_compile_options(${target} PRIVATE -fPIC -fno-strict-aliasing)
    endif()
endfunction()

function(jaymod_copy_alt_name target alt_name)
    if(WIN32 AND JAYMOD_ALT_ARCH)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${target}>"
                "$<TARGET_FILE_DIR:${target}>/${alt_name}"
            COMMENT "Also emit ${alt_name} for older ETL loaders"
        )
    endif()
endfunction()

add_library(qagame SHARED
    ${LUA_SRCS}
    ${GAME_SRCS}
    ${BGAME_SRCS}
    ${BGAME_PLATFORM_SRCS}
    ${BASE_SRCS}
    ${OMNIBOT_SRCS}
    ${OMNIBOT_PLATFORM_SRCS}
)
set_target_properties(qagame PROPERTIES OUTPUT_NAME "${QAGAME_OUTPUT_NAME}")
if(WIN32 AND MSVC)
    target_sources(qagame PRIVATE src/game/game.def)
endif()
if(WIN32)
    jaymod_configure_module(qagame "GAMEDLL;FEATURE_LUA" "winmm;ws2_32;iphlpapi;winhttp")
else()
    jaymod_configure_module(qagame "GAMEDLL;FEATURE_LUA" "")
endif()
jaymod_copy_alt_name(qagame "qagame_mp_x86_64.dll")

add_library(cgame SHARED
    ${LUA_SRCS}
    ${CGAME_SRCS}
    ${CGAME_PLATFORM_SRCS}
    ${CGAME_BGAME_SRCS}
    ${BGAME_PLATFORM_SRCS}
    ${BASE_SRCS}
    src/ui/ui_shared.cpp
)
set_target_properties(cgame PROPERTIES OUTPUT_NAME "${CGAME_OUTPUT_NAME}")
if(WIN32 AND MSVC)
    target_sources(cgame PRIVATE src/cgame/cgame.def)
endif()
if(WIN32)
    jaymod_configure_module(cgame "CGAMEDLL" "winmm;ws2_32;iphlpapi;advapi32")
else()
    jaymod_configure_module(cgame "CGAMEDLL" "")
endif()
jaymod_copy_alt_name(cgame "cgame_mp_x86_64.dll")

add_library(ui SHARED
    ${LUA_SRCS}
    ${UI_SRCS}
    ${UI_BGAME_SRCS}
    ${BGAME_PLATFORM_SRCS}
    ${BASE_SRCS}
)
set_target_properties(ui PROPERTIES OUTPUT_NAME "${UI_OUTPUT_NAME}")
if(WIN32 AND MSVC)
    target_sources(ui PRIVATE src/ui/ui.def)
endif()
if(WIN32)
    jaymod_configure_module(ui "UIDLL" "winmm")
else()
    jaymod_configure_module(ui "UIDLL" "")
endif()
jaymod_copy_alt_name(ui "ui_mp_x86_64.dll")

install(TARGETS qagame cgame ui
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
)
