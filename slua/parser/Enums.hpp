/*
** See Copyright Notice inside Include.hpp
*/
#pragma once

#include <cstdint>

//https://www.lua.org/manual/5.4/manual.html
//https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form
//https://www.sciencedirect.com/topics/computer-science/backus-naur-form

namespace sluaParse
{

	enum class BinOpType : uint8_t
	{
		NONE,

		ADD,            // "+"
		SUBTRACT,       // "-"
		MULTIPLY,       // "*"
		DIVIDE,         // "/"
		FLOOR_DIVIDE,   // "//"
		EXPONENT,       // "^"
		MODULO,         // "%"
		BITWISE_AND,    // "&"
		BITWISE_XOR,    // "~"
		BITWISE_OR,     // "|"
		SHIFT_RIGHT,    // ">>"
		SHIFT_LEFT,     // "<<"
		CONCATENATE,    // ".."
		LESS_THAN,      // "<"
		LESS_EQUAL,     // "<="
		GREATER_THAN,   // ">"
		GREATER_EQUAL,  // ">="
		EQUAL,          // "=="
		NOT_EQUAL,      // "~="
		LOGICAL_AND,    // "and"
		LOGICAL_OR,     // "or"
	};

	enum class UnOpType : uint8_t
	{
		NONE,

		NEGATE,        // "-"
		LOGICAL_NOT,   // "not"
		LENGTH,        // "#"
		BITWISE_NOT,   // "~"

		//Slua
		ALLOCATE,		// "alloc"
		DEREF,			// "*"

		TO_REF,			// "&"
		TO_REF_MUT,		// "&mut"
		TO_PTR_CONST,	// "*const"
		TO_PTR_MUT		// "*mut"
	};

	enum class PostUnOpType : uint8_t
	{
		NONE,

		PROPOGATE_ERR, // "?"
	};
}
