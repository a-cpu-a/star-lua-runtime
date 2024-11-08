/*
** See Copyright Notice inside Include.hpp
*/
#pragma once

#include <type_traits>

namespace slua
{
	//Internal, used bc, const T != T, etc
	template<typename T>
	using _RawType = std::remove_cv_t<std::remove_reference_t<T>>;


	//Internal, used bc, const T != T, etc
	template<typename T>
	struct _ToLua
	{
		using Type = T;
	};

	template<typename T>
	using ToLua = _ToLua<_RawType<T>>::Type;




	template<typename T>
	concept LuaType = std::is_same_v<_RawType<T>, slua::ToLua<T>>;
	template<typename T>
	concept NonLuaType = !std::is_same_v<_RawType<T>, slua::ToLua<T>>;

	template<typename T>
	concept PushableLuaType = requires(slua::ToLua<T> t) {
		{ t.push(nullptr, t) } -> std::same_as<int>;
	};
}

// MUSN'T be inside a namespace !!!
// 
// the ... means template args
//
#define SLua_MAP_TYPE(_NORMAL_TYPE,_WRAPPER,...) namespace slua { template<__VA_ARGS__>struct _ToLua<_NORMAL_TYPE> {using Type = _WRAPPER;}; }