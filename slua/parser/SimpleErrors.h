/*
** See Copyright Notice inside Include.hpp
*/
#pragma once

#include <cstdint>
#include <luaconf.h>

//https://www.lua.org/manual/5.4/manual.html
//https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form
//https://www.sciencedirect.com/topics/computer-science/backus-naur-form

#include <slua/parser/State.hpp>
#include <slua/parser/Input.hpp>

namespace sluaParse
{

	inline std::string errorLocStr(AnyInput auto& in) {
		return " " + in.fileName() + " (" + std::to_string(in.getLoc().line) + "):" + std::to_string(in.getLoc().index);
	}

	struct UnexpectedCharacterError : std::exception
	{
		std::string m;
		const char* what() const { return m.c_str(); }
	};
	struct UnexpectedFileEndError : std::exception
	{
		std::string m;
		const char* what() const { return m.c_str(); }
	};
	struct ReservedNameError : std::exception
	{
		std::string m;
		const char* what() const { return m.c_str(); }
	};
}