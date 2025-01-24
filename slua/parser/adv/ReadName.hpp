/*
** See Copyright Notice inside Include.hpp
*/
#pragma once

#include <cstdint>
#include <unordered_set>

//https://www.lua.org/manual/5.4/manual.html
//https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form
//https://www.sciencedirect.com/topics/computer-science/backus-naur-form

#include <slua/parser/State.hpp>
#include <slua/parser/Input.hpp>
#include <slua/parser/adv/SkipSpace.hpp>
#include <slua/parser/adv/RequireToken.hpp>

namespace sluaParse
{
	constexpr bool isValidNameStartChar(const char c) {
		// Check if the character is in the range of 'A' to 'Z' or 'a' to 'z', or '_'
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c=='_';
	}
	constexpr bool isValidNameChar(const char c) {
		// Check if the character is in the range of '0' to '9', 'A' to 'Z' or 'a' to 'z', or '_'
		return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c=='_';
	}
	inline std::string readName(AnyInput auto& in, const bool allowError = false)
	{
		/*
		Names (also called identifiers) in Lua can be any string
		of Latin letters, Arabic-Indic digits, and underscores,
		not beginning with a digit and not being a reserved word.

		The following keywords are reserved and cannot be used as names:

		 and       break     do        else      elseif    end
		 false     for       function  goto      if        in
		 local     nil       not       or        repeat    return
		 then      true      until     while
		*/
		skipSpace(in);

		if (!in)
			throw UnexpectedFileEndError("Expected identifier/name: but file ended" + errorLocStr(in));


		static const std::unordered_set<std::string> reservedKeywords = {
			"and", "break", "do", "else", "elseif", "end", "false", "for", "function",
			"goto", "if", "in", "local", "nil", "not", "or", "repeat", "return",
			"then", "true", "until", "while"
		};

		const uint8_t firstChar = in.peek();

		// Ensure the first character is valid (a letter or underscore)
		if (!isValidNameStartChar(firstChar))
		{
			if (allowError)
				return "";
			throw UnexpectedCharacterError("Invalid identifier/name start: must begin with a letter or underscore" + errorLocStr(in));
		}


		std::string res = in.get(); // Consume the first valid character

		// Consume subsequent characters (letters, digits, or underscores)
		while (in)
		{
			const uint8_t ch = in.peek();
			if (isValidNameChar(ch))
			{
				res += in.get();
				continue;
			}
			break; // Stop when a non-identifier character is found
		}

		// Check if the resulting string is a reserved keyword
		if (reservedKeywords.find(res) != reservedKeywords.end())
		{
			if (allowError)
				return "";
			throw ReservedNameError("Invalid identifier: matches a reserved keyword" + errorLocStr(in));
		}

		return res;
	}

	//uhhh, dont use?
	inline NameList readNames(AnyInput auto& in, const bool requires1 = true)
	{
		NameList res;

		if (requires1)
			res.push_back(readName(in));

		bool skipComma = !requires1;//comma wont exist if the first one doesnt exist
		bool allowNameError = !requires1;//if the first one doesnt exist

		while (skipComma || checkReadToken(in, ','))
		{
			skipComma = false;// Only skip first comma

			const std::string str = readName(in, allowNameError);

			if (allowNameError && str.empty())
				return {};//no names

			res.push_back(str);

			allowNameError = false;//not the first one anymore
		}
		return res;
	}
}