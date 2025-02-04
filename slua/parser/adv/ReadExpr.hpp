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
#include <slua/parser/adv/ReadNumeral.h>
#include <slua/parser/basic/ReadOperators.hpp>

namespace sluaParse
{
	inline Expression readExpr(AnyInput auto& in)
	{
		/*
			nil | false | true | Numeral | LiteralString | �...� | functiondef
			| prefixexp | tableconstructor | exp binop exp | unop exp
		*/

		const Position startPos = in.getLoc();

		bool isNilIntentional = false;
		Expression basicRes;
		basicRes.place = startPos;
		basicRes.unOp = readOptUnOp(in);

		skipSpace(in);

		const char firstChar = in.peek();
		switch (firstChar)
		{
		case 'n':
			if (checkReadTextToken(in, "nil"))
			{
				basicRes.data = ExprType::NIL();
				isNilIntentional = true;
				break;
			}
			break;
		case 'f':

			if (checkReadTextToken(in, "false")) { basicRes.data = ExprType::FALSE(); break; }
			if (checkReadTextToken(in, "function")) { basicRes.data = ExprType::FUNCTION_DEF(readFuncBody(in)); break; }
			break;
		case 't':
			if (checkReadTextToken(in, "true")) { basicRes.data = ExprType::TRUE(); break; }
			break;
		case '.':
			if (checkReadToken(in, "..."))
			{
				basicRes.data = ExprType::VARARGS();
				break;
			}
			//break;
			[[fallthrough]];//handle as numeral instead (.0123, etc)
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			basicRes.data = readNumeral(in,firstChar);
			break;
		case '"':
		case '\'':
		case '[':
			basicRes.data = ExprType::LITERAL_STRING(readStringLiteral(in, firstChar));
			break;
		case '{':
			basicRes.data = ExprType::TABLE_CONSTRUCTOR(readTableConstructor(in));
			break;
		}

		if (!isNilIntentional && std::holds_alternative<ExprType::NIL>(basicRes.data)
			&&(firstChar=='(' || isValidNameStartChar(firstChar))
			)
		{//Prefix expr! or func-call

			Var varData;
			std::vector<ArgFuncCall> funcCallData;// Current func call chain, empty->no chain
			bool varDataNeedsSubThing = false;

			parseVarBase(in, firstChar, varData, varDataNeedsSubThing);

			//This requires manual parsing, and stuff (at every step, complex code)
			while (true)
			{
				skipSpace(in);

				const char opType = in.peek();
				switch (opType)
				{
				case ':'://Self funccall
					in.skip();
					std::string name = readName(in);

					funcCallData.emplace_back(name, readArgs(in));
					break;
				case '{':
				case '"':
				case '('://Funccall
					funcCallData.emplace_back("", readArgs(in));
					break;
				case '.':// Index
				{
					in.skip();

					SubVarType::NAME res{};
					res.funcCalls = std::move(funcCallData);// Move auto-clears it
					res.idx = readName(in);

					varDataNeedsSubThing = false;
					varData.sub.emplace_back(res);
					break;
				}
				case '[':// Arr-index
				{
					SubVarType::EXPR res{};
					res.funcCalls = std::move(funcCallData);// Move auto-clears it

					in.skip();
					res.idx = readExpr(in);
					requireToken(in, "]");

					varDataNeedsSubThing = false;
					varData.sub.emplace_back(res);
					break;
				}
				default:
				{
					if (funcCallData.empty())
					{
						if (varDataNeedsSubThing)
						{
							Expression res;
							res = std::move(std::get<BaseVarType::EXPR>(varData.base));
							return ExprType::LIM_PREFIX_EXP(new LimPrefixExpr(res));
						}
						return ExprType::LIM_PREFIX_EXP(new LimPrefixExpr(varData));
					}
					if (varDataNeedsSubThing)
					{
						BaseVarType::EXPR& bVarExpr = std::get<BaseVarType::EXPR>(varData.base);
						return FuncCall(LimPrefixExprType::EXPR(std::move(bVarExpr.start)), funcCallData);
					}
					return FuncCall(LimPrefixExprType::VAR(std::move(varData)), funcCallData);
				}
				}
			}
		}
		//check bin op


		const BinOpType firstBinOp = readOptBinOp(in);

		if (firstBinOp == BinOpType::NONE)
			return basicRes;

		ExprType::MULTI_OPERATION resData{};

		resData.first = std::make_unique(basicRes);
		resData.extra.emplace_back(firstBinOp, readExpr(in));

		while (true)
		{
			const BinOpType binOp = readOptBinOp(in);

			if (binOp == BinOpType::NONE)
				break;

			resData.extra.emplace_back(binOp, readExpr(in));
		}
		Expression ret;
		ret.place = startPos;
		ret.data = resData;

		return ret;
	}

	inline ExpList readExpList(AnyInput auto& in)
	{
		/*
			explist ::= exp {�,� exp}
		*/
		ExpList ret{};
		ret.push_back(readExpr(in));

		while (checkReadToken(in, ","))
		{
			ret.push_back(readExpr(in));
		}
		return ret;
	}
}