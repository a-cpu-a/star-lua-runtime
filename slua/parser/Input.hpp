/*
** See Copyright Notice inside Include.hpp
*/
#pragma once

#include <string>
#include <span>

//https://www.lua.org/manual/5.4/manual.html
//https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form
//https://www.sciencedirect.com/topics/computer-science/backus-naur-form

#include "State.hpp"

namespace sluaParse
{
	struct EndOfStreamError : std::exception
	{
		const char* what() const { return "End of stream error"; }
	};

	struct Input
	{
		std::string fName;
		size_t curLine = 0;
		size_t curLinePos = 0;

		std::span<const uint8_t> text;
		size_t idx=0;

		uint8_t peek()
		{
			if (idx >= text.size())
				throw EndOfStreamError();

			return text[idx];
		}

		//offset 0 is the same as peek()
		uint8_t peekAt(const size_t offset)
		{
			if (idx + offset >= text.size()
				|| idx > SIZE_MAX - offset)	//idx + offset overflows, so...
				throw EndOfStreamError();

			return text[idx + offset];
		}

		// span must be valid until next get(), so, any other peek()'s must not invalidate these!!!
		std::span<const uint8_t> peek(const size_t count)
		{
			if (idx + count > text.size()	//position after this peek() can be at text.size(), but not above it
				|| idx > SIZE_MAX - count)	//idx + count overflows, so...
				throw EndOfStreamError();

			std::span<const uint8_t> res = text.subspan(idx, count);

			return res;
		}
		void skip(const size_t count = 1)
		{
			//if (idx >= text.size())
			//	throw EndOfStreamError();

			curLinePos += count;
			idx += count;
		}

		uint8_t get()
		{
			if (idx >= text.size())
				throw EndOfStreamError();

			curLinePos++;
			return text[idx++];
		}
		// span must be valid until next get(), so, any peek()'s must not invalidate these!!!
		std::span<const uint8_t> get(const size_t count)
		{
			if (idx + count > text.size()	//position after this get() can be at text.size(), but not above it
				|| idx > SIZE_MAX - count)	//idx + count overflows, so...
				throw EndOfStreamError();

			std::span<const uint8_t> res = text.subspan(idx, count);

			idx += count;
			curLinePos += count;

			return res;
		}

		/* Returns true, while stream still has stuff */
		operator bool() const {
			return idx < text.size();
		}
		//Passing 0 is the same as (!in)
		bool checkOOB(const size_t offset) const {
			return (
				idx + offset >= text.size()
				|| idx > SIZE_MAX - offset
				);
		}


		//Error output

		std::string fileName() const {
			return fName;
		}
		Position getLoc() const {
			return { curLine,curLinePos };
		}
		void newLine() {
			curLine++;
			curLinePos = 0;
		}

	};

	//Here, so streamed inputs can be made
	template<class T = Input>
	concept AnyInput = requires(T t) {
		{ t.skip() } -> std::same_as<void>;
		{ t.skip((size_t)100) } -> std::same_as<void>;

		{ t.get() } -> std::same_as<uint8_t>;
		{ t.get((size_t)100) } -> std::same_as<std::span<const uint8_t>>;

		{ t.peek() } -> std::same_as<uint8_t>;
		{ t.peekAt((size_t)100) } -> std::same_as<uint8_t>;
		{ t.peek((size_t)100) } -> std::same_as<std::span<const uint8_t>>;


		/* Returns true, while stream still has stuff */
		//{ (bool)t } -> std::same_as<bool>; //Crashes intelisense


		{ t.checkOOB((size_t)100) } -> std::same_as<bool>;


		//Error output

		{ t.fileName() } -> std::same_as<std::string>;
		{ t.getLoc() } -> std::same_as<Position>;

		//Management
		{ t.newLine() } -> std::same_as<void>;
	};
}