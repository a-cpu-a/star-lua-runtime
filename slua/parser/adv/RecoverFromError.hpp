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
#include <slua/parser/adv/ReadStringLiteral.hpp>

namespace sluaParse
{
	template<size_t TOK_SIZE>
	bool recoverErrorTextToken(AnyInput auto& in, const char(&tok)[TOK_SIZE])
	{
		while (in)
		{
			const char ch = in.peek();
			switch (ch)
			{
			case ' ':
			case '\n':
			case '\r':
			case '\t':
			case '\f':
			case '\v':
				skipSpace(in);
				break;
			case '"':
			case '\'':
				readStringLiteral(in, ch);

			case '[':
			{
				const char ch1 = in.peekAt(1);
				if (ch1 == '=' || ch1 == '[')
				{
					readStringLiteral(in, ch);
				}
				break;
			}
			default:
				break;
			}

			if (checkTextToken(in, tok))
			{// Found it, recovered!
				return true;
			}
			in.skip();//Not found, try at next char
		}
		return false;
	}
}