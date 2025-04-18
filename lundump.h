/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ifndef lundump_h
#define lundump_h

#include <limits.h>

#include "llimits.h"
#include "lobject.h"
#include "lzio.h"


/* data to catch conversion errors */
static const char LUAC_DATA[] = "\x19\x93\r\n\x1a\n";

constexpr inline int LUAC_INT	        = 0x5678;
constexpr inline lua_Number LUAC_NUM	= cast_num(370.5);

/*
** Encode major-minor version in one byte, one nibble for each
*/
constexpr inline int LUAC_VERSION = (LUA_VERSION_MAJOR_N * 16 + LUA_VERSION_MINOR_N);

constexpr inline int LUAC_FORMAT = 0;	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, const char* name,
                                               int fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC int luaU_dump (lua_State* L, const Proto* f, lua_Writer w,
                         void* data, int strip);

#endif
