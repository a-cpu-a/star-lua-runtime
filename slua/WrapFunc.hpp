/*
** See Copyright Notice inside Include.hpp
*/
#pragma once

#include <slua/types/Converter.hpp>
#include <slua/types/ReadWrite.hpp>
#include <slua/types/basic/Lua.hpp>

namespace slua
{

	template<typename RET_T>
	using Args0CppFunc = RET_T(*)();
	template<typename RET_T, typename IN1_T>
	using Args1CppFunc = RET_T(*)(IN1_T&);

	template<typename RET_T, typename IN1_T, typename IN2_T>
	using Args2CppFunc = RET_T(*)(IN1_T&, IN2_T&);
	template<typename RET_T, typename IN1_T, typename IN2_T, typename IN3_T>
	using Args3CppFunc = RET_T(*)(IN1_T&, IN2_T&, IN3_T&);
	template<typename RET_T, typename IN1_T, typename IN2_T, typename IN3_T, typename IN4_T>
	using Args4CppFunc = RET_T(*)(IN1_T&, IN2_T&, IN3_T&, IN4_T&);
	template<typename RET_T, typename IN1_T, typename IN2_T, typename IN3_T, typename IN4_T, typename IN5_T>
	using Args5CppFunc = RET_T(*)(IN1_T&, IN2_T&, IN3_T&, IN4_T&, IN5_T&);
	template<typename RET_T, typename IN1_T, typename IN2_T, typename IN3_T, typename IN4_T, typename IN5_T, typename IN6_T>
	using Args6CppFunc = RET_T(*)(IN1_T&, IN2_T&, IN3_T&, IN4_T&, IN5_T&, IN6_T&);


#define _SLua_REQUIRE_ARGS(_COUNT) \
	if(lua_gettop(L)!=_COUNT) \
		return slua::error(L, LUACC_FUNCTION "Function" \
			LUACC_STRING_SINGLE " '" LUACC_DEFAULT + funcName + LUACC_STRING_SINGLE "'" LUACC_DEFAULT \
			" needs " LUACC_NUMBER #_COUNT LUACC_ARGUMENT " arguments" LUACC_DEFAULT ".")

#define _SLua_CHECK_ARG(_ARG) \
	const bool arg_ ## _ARG ## _res = slua::checkThrowing<IN ## _ARG ## _T>(L, _ARG); \
	if (!arg_ ## _ARG ## _res) \
		return slua::error(L, LUACC_ARGUMENT "Argument " \
			LUACC_NUMBER #_ARG LUACC_DEFAULT " of " LUACC_FUNCTION "function" \
			LUACC_STRING_SINGLE " '" LUACC_DEFAULT + funcName + LUACC_STRING_SINGLE "'" LUACC_DEFAULT \
			", is " LUACC_INVALID "not" LUACC_DEFAULT " a " \
			LUACC_STRING_SINGLE "'" LUACC_DEFAULT + slua::getName<IN ## _ARG ## _T>() + LUACC_STRING_SINGLE "'" \
		); /*\
	if(arg_ ## _ARG ## _res>0) return arg_ ## _ARG ## _res*/ //why did this exist, lol

#define _SLua_READ_ARG(_ARG) IN ## _ARG ## _T in ## _ARG = slua::read<IN ## _ARG ## _T>(L, _ARG)
#define _SLua_CHECK_N_READ_ARG(_ARG) _SLua_CHECK_ARG(_ARG); _SLua_READ_ARG(_ARG)

#define _SLua_RUN_CPP_FUNC(_VAR,...) const slua::ToLua<RET_T> _VAR = func(__VA_ARGS__)
#define _SLua_RETURN_VALUE(_VAR)  return slua::ToLua<RET_T>::push(L,_VAR)

#define _SLua_RUN_N_RETURN(...) _SLua_RUN_CPP_FUNC(ret,__VA_ARGS__); _SLua_RETURN_VALUE(ret)

	template <typename RET_T>
	inline int runCppFunc(lua_State* L, const std::string& funcName, Args0CppFunc<RET_T> func)
	{
		_SLua_REQUIRE_ARGS(0);
		_SLua_RUN_N_RETURN();
	}
	template <typename RET_T, typename IN1_T>
	inline int runCppFunc(const std::string& funcName, Args1CppFunc< RET_T, IN1_T> func, lua_State* L)
	{
		if (lua_gettop(L) != 1)
			return slua::error(L, LUACC_FUNCTION "Function"
				LUACC_STRING_SINGLE " '" LUACC_DEFAULT + funcName + LUACC_STRING_SINGLE "'" LUACC_DEFAULT
				" needs " LUACC_NUMBER "1" LUACC_ARGUMENT " argument" LUACC_DEFAULT "."
			);

		_SLua_CHECK_N_READ_ARG(1);

		_SLua_RUN_N_RETURN(in1);
	}

	template <typename RET_T, typename IN1_T, typename IN2_T>
	inline int runCppFunc(lua_State* L, const std::string& funcName, Args2CppFunc<RET_T, IN1_T, IN1_T> func)
	{
		_SLua_REQUIRE_ARGS(2);

		_SLua_CHECK_N_READ_ARG(1);
		_SLua_CHECK_N_READ_ARG(2);

		_SLua_RUN_N_RETURN(in1, in2);
	}
	template <typename RET_T, typename IN1_T, typename IN2_T, typename IN3_T>
	inline int runCppFunc(lua_State* L, const std::string& funcName, Args3CppFunc<RET_T, IN1_T, IN1_T, IN3_T> func)
	{
		_SLua_REQUIRE_ARGS(3);

		_SLua_CHECK_N_READ_ARG(1);
		_SLua_CHECK_N_READ_ARG(2);
		_SLua_CHECK_N_READ_ARG(3);

		_SLua_RUN_N_RETURN(in1, in2, in3);
	}
	template <typename RET_T, typename IN1_T, typename IN2_T, typename IN3_T, typename IN4_T>
	inline int runCppFunc(lua_State* L, const std::string& funcName, Args4CppFunc<RET_T, IN1_T, IN1_T, IN3_T, IN4_T> func)
	{
		_SLua_REQUIRE_ARGS(4);

		_SLua_CHECK_N_READ_ARG(1);
		_SLua_CHECK_N_READ_ARG(2);
		_SLua_CHECK_N_READ_ARG(3);
		_SLua_CHECK_N_READ_ARG(4);

		_SLua_RUN_N_RETURN(in1, in2, in3, in4);
	}
	template <typename RET_T, typename IN1_T, typename IN2_T, typename IN3_T, typename IN4_T, typename IN5_T>
	inline int runCppFunc(lua_State* L, const std::string& funcName, Args5CppFunc<RET_T, IN1_T, IN1_T, IN3_T, IN4_T, IN5_T> func)
	{
		_SLua_REQUIRE_ARGS(5);

		_SLua_CHECK_N_READ_ARG(1);
		_SLua_CHECK_N_READ_ARG(2);
		_SLua_CHECK_N_READ_ARG(3);
		_SLua_CHECK_N_READ_ARG(4);
		_SLua_CHECK_N_READ_ARG(5);

		_SLua_RUN_N_RETURN(in1, in2, in3, in4, in5);
	}
	template <typename RET_T, typename IN1_T, typename IN2_T, typename IN3_T, typename IN4_T, typename IN5_T, typename IN6_T>
	inline int runCppFunc(lua_State* L, const std::string& funcName, Args6CppFunc<RET_T, IN1_T, IN1_T, IN3_T, IN4_T, IN5_T, IN6_T> func)
	{
		_SLua_REQUIRE_ARGS(6);

		_SLua_CHECK_N_READ_ARG(1);
		_SLua_CHECK_N_READ_ARG(2);
		_SLua_CHECK_N_READ_ARG(3);
		_SLua_CHECK_N_READ_ARG(4);
		_SLua_CHECK_N_READ_ARG(5);
		_SLua_CHECK_N_READ_ARG(6);

		_SLua_RUN_N_RETURN(in1, in2, in3, in4, in5, in6);
	}

	inline int runCppFuncWrapped(lua_State* L, const std::string& funcName, auto func)
	{
		try
		{
			runCppFunc(L, funcName, func);
		}
		catch (const slua::Error& e)
		{
			return slua::error(L, LUACC_FUNCTION "Function"
				LUACC_STRING_SINGLE " '" LUACC_DEFAULT + funcName + LUACC_STRING_SINGLE "'" LUACC_DEFAULT
				" had a " LUACC_INVALID "error" LUACC_DEFAULT ": "
				, e);
		}
	}



	// Wrap a C++ into a lua function, ment for lua_pushcfunction
#define SLua_WrapRaw(_LUA_NAME,_CPP_FUNC) [](lua_State* L){return ::slua::runCppFuncWrapped(L,_LUA_NAME,_CPP_FUNC);}
	// Wrap a C++ into a lua function, name pair, for your library function tables
#define SLua_Wrap(_LUA_NAME,_CPP_FUNC) {_LUA_NAME,SLua_WrapRaw(_LUA_NAME,_CPP_FUNC)}




#define _SLua_MULTI_WRAP_CASE(_LUA_NAME,_COUNT,_CPP_FUNC) case _COUNT:return ::slua::runCppFuncWrapped(L,_LUA_NAME "(" #_COUNT ")",_CPP_FUNC)


	// Wrap 2 C++ functions overloaded by arg count into 1 lua function, ment for lua_pushcfunction
	// _C1 is the argument count of _CPP_FUNC
	// _C2 is the argument count of _CPP_FUNC2
#define SLua_Wrap2Raw(_LUA_NAME,_C1,_CPP_FUNC,_C2,_CPP_FUNC2) [](lua_State* L){switch(lua_gettop(L)){ \
		_SLua_MULTI_WRAP_CASE(_LUA_NAME,_C1,_CPP_FUNC); \
		_SLua_MULTI_WRAP_CASE(_LUA_NAME,_C2,_CPP_FUNC2); \
		default: return ::slua::error(L,LUACC_FUNCTION "Function" \
			LUACC_STRING_SINGLE " '" LUACC_DEFAULT _LUA_NAME LUACC_STRING_SINGLE "'" LUACC_DEFAULT \
			" needs " LUACC_NUMBER #_C1 LUACC_DEFAULT " or " LUACC_NUMBER #_C2 " " LUACC_ARGUMENT "arguments" LUACC_DEFAULT "." );} }

	// Wrap 2 C++ functions overloaded by arg count into 1 lua function, name pair, for your library function tables
	// _C1 is the argument count of _CPP_FUNC
	// _C2 is the argument count of _CPP_FUNC2
#define SLua_Wrap2(_LUA_NAME,_C1,_CPP_FUNC,_C2,_CPP_FUNC2) {_LUA_NAME,SLua_Wrap2Raw(_LUA_NAME,_C1,_CPP_FUNC,_C2,_CPP_FUNC2)}


	// Wrap 3 C++ functions overloaded by arg count into 1 lua function, ment for lua_pushcfunction
	// _C1 is the argument count of _CPP_FUNC
	// _C2 is the argument count of _CPP_FUNC2
	// _C3 is the argument count of _CPP_FUNC3
#define SLua_Wrap3Raw(_LUA_NAME,_C1,_CPP_FUNC,_C2,_CPP_FUNC2,_C3,_CPP_FUNC3) [](lua_State* L){switch(lua_gettop(L)){ \
		_SLua_MULTI_WRAP_CASE(_LUA_NAME,_C1,_CPP_FUNC); \
		_SLua_MULTI_WRAP_CASE(_LUA_NAME,_C2,_CPP_FUNC2); \
		_SLua_MULTI_WRAP_CASE(_LUA_NAME,_C3,_CPP_FUNC3); \
		default: return ::slua::error(L,LUACC_FUNCTION "Function" \
			LUACC_STRING_SINGLE " '" LUACC_DEFAULT _LUA_NAME LUACC_STRING_SINGLE "'" LUACC_DEFAULT \
			" needs " LUACC_NUMBER #_C1 LUACC_DEFAULT \
			", " LUACC_NUMBER #_C2 LUACC_DEFAULT \
			" or " LUACC_NUMBER #_C3 " " LUACC_ARGUMENT "arguments" LUACC_DEFAULT "." );} }

	// Wrap 3 C++ functions overloaded by arg count into 1 lua function, name pair, for your library function tables
	// _C1 is the argument count of _CPP_FUNC
	// _C2 is the argument count of _CPP_FUNC2
	// _C3 is the argument count of _CPP_FUNC3
#define SLua_Wrap3(_LUA_NAME,_C1,_CPP_FUNC,_C2,_CPP_FUNC2,_C3,_CPP_FUNC3) {_LUA_NAME,SLua_Wrap3Raw(_LUA_NAME,_C1,_CPP_FUNC,_C2,_CPP_FUNC2,_C3,_CPP_FUNC3)}


	// Wrap 4 C++ functions overloaded by arg count into 1 lua function, ment for lua_pushcfunction
	// _C1 is the argument count of _CPP_FUNC
	// _C2 is the argument count of _CPP_FUNC2
	// _C3 is the argument count of _CPP_FUNC3
	// _C4 is the argument count of _CPP_FUNC4
#define SLua_Wrap4Raw(_LUA_NAME,_C1,_CPP_FUNC,_C2,_CPP_FUNC2,_C3,_CPP_FUNC3,_C4,_CPP_FUNC4) [](lua_State* L){switch(lua_gettop(L)){ \
		_SLua_MULTI_WRAP_CASE(_LUA_NAME,_C1,_CPP_FUNC); \
		_SLua_MULTI_WRAP_CASE(_LUA_NAME,_C2,_CPP_FUNC2); \
		_SLua_MULTI_WRAP_CASE(_LUA_NAME,_C3,_CPP_FUNC3); \
		_SLua_MULTI_WRAP_CASE(_LUA_NAME,_C4,_CPP_FUNC4); \
		default: return ::slua::error(L,LUACC_FUNCTION "Function" \
			LUACC_STRING_SINGLE " '" LUACC_DEFAULT _LUA_NAME LUACC_STRING_SINGLE "'" LUACC_DEFAULT \
			" needs " LUACC_NUMBER #_C1 LUACC_DEFAULT \
			", " LUACC_NUMBER #_C2 LUACC_DEFAULT \
			", " LUACC_NUMBER #_C3 LUACC_DEFAULT \
			" or " LUACC_NUMBER #_C4 " " LUACC_ARGUMENT "arguments" LUACC_DEFAULT "." );} }

	// Wrap 4 C++ functions overloaded by arg count into 1 lua function, name pair, for your library function tables
	// _C1 is the argument count of _CPP_FUNC
	// _C2 is the argument count of _CPP_FUNC2
	// _C3 is the argument count of _CPP_FUNC3
	// _C4 is the argument count of _CPP_FUNC4
#define SLua_Wrap4(_LUA_NAME,_C1,_CPP_FUNC,_C2,_CPP_FUNC2,_C3,_CPP_FUNC3,_C4,_CPP_FUNC4) {_LUA_NAME,SLua_Wrap4Raw(_LUA_NAME,_C1,_CPP_FUNC,_C2,_CPP_FUNC2,_C3,_CPP_FUNC3,_C4,_CPP_FUNC4)}
}