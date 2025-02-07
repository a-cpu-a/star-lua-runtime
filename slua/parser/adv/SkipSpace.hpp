/*
** See Copyright Notice inside Include.hpp
*/
#pragma once

#include <cstdint>

//https://www.lua.org/manual/5.4/manual.html
//https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form
//https://www.sciencedirect.com/topics/computer-science/backus-naur-form

#include <slua/parser/Input.hpp>

namespace sluaParse
{
	constexpr bool isSpaceChar(const char ch)
	{
		return ch == ' ' || ch == '\f' || ch == '\t' || ch == '\v' || ch == '\n' || ch == '\r';
	}
	enum class ParseNewlineState : uint8_t
	{
		NONE,
		CARI,
	};

	inline void manageNewlineState(const char ch, ParseNewlineState& nlState, AnyInput auto& in)
	{
		switch (nlState)
		{
		case sluaParse::ParseNewlineState::NONE:
			if (ch == '\n')
				in.newLine();
			else if (ch == '\r')
				nlState = sluaParse::ParseNewlineState::CARI;
			break;
		case sluaParse::ParseNewlineState::CARI:
			if (ch != '\r')
			{//  \r\n, or \r(normal char)
				in.newLine();
				nlState = sluaParse::ParseNewlineState::NONE;
			}
			else// \r\r
				in.newLine();
			break;
		}
	}

	inline void skipSpace(AnyInput auto& in)
	{
		/*

		Lua is a free-form language.
		It ignores spaces and comments between lexical
		elements (tokens), except as delimiters between
		two tokens.


		Multiline comments:

		We define an opening long bracket of level n as
		an opening square bracket followed by n equal
		signs followed by another opening square bracket.

		So, an opening long bracket of level 0 is written
		as [[, an opening long bracket of level 1 is
		written as [=[, and so on.

		A closing long bracket is defined similarly; for
		instance, a closing long bracket of level 4 is
		written as ]====].

		A long literal starts with
		an opening long bracket of any level and ends at
		the first closing long bracket of the same level.

		It can contain any text except a closing bracket
		of the same level.

		Literals in this bracketed form can run for
		several lines, do not interpret any escape
		sequences, and ignore long brackets of
		any other level.

		*/

		bool insideLineComment = false;
		ParseNewlineState nlState = ParseNewlineState::NONE;

		bool insideMultilineComment = false;
		size_t multilineCommentLevel = SIZE_MAX;

		while (in)
		{
			const uint8_t ch = in.peek();
			manageNewlineState(ch, nlState, in);

			if (insideLineComment)
			{
				if (ch == '\n' || ch == '\r')
				{
					insideLineComment = false;//new line, so exit comment
				}

				in.skip();
				continue;
			}

			// Handle inside-multiline-comment scenario
			else if (insideMultilineComment)
			{
				if (ch == ']') // Check for possible multiline comment closing
				{
					size_t level = 0;
					while (in.peekAt(1 + level) == '=') // Count '=' signs
						level++;

					if (in.peekAt(1 + level) == ']' && level == multilineCommentLevel)
					{
						insideMultilineComment = false;
						in.skip(2 + level); // Skip closing bracket
						continue;
					}
				}
				in.skip(); // Skip other characters in multiline comment
				continue;
			}

			/*

			In source code, Lua recognizes as
			spaces the standard ASCII whitespace characters
			space, form feed, newline, carriage return,
			horizontal tab, and vertical tab.

			*/

			if (isSpaceChar(ch))
			{
				in.skip();
				continue;
			}

			if (ch == '-')
			{//maybe comment?
				const uint8_t nextCh = in.peekAt(1);
				if (nextCh == '-') // Single-line or multiline comment starts
				{
					const uint8_t nextNextCh = in.peekAt(2);
					if (nextNextCh == '[') // Possible multiline comment
					{
						size_t level = 0;
						while (in.peekAt(3 + level) == '=') // Count '=' signs
							level++;

						if (in.peekAt(3 + level) == '[') // Confirm multiline comment start
						{
							insideMultilineComment = true;
							multilineCommentLevel = level;
							in.skip(4 + level); // Skip "--[=["
							continue;
						}
					}

					insideLineComment = true; // Otherwise, it's a single-line comment
					in.skip(2); // Skip "--"
					continue;
				}
			}
			break;
		}
	}
}