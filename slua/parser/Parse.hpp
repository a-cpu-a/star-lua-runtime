/*
** See Copyright Notice inside Include.hpp
*/
#pragma once

#include <cstdint>
#include <unordered_set>

//https://www.lua.org/manual/5.4/manual.html
//https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form
//https://www.sciencedirect.com/topics/computer-science/backus-naur-form

#include <slua/parser/Input.hpp>
#include <slua/parser/State.hpp>

#include "basic/ReadMiscNames.hpp"
#include "adv/ReadName.hpp"
#include "adv/RequireToken.hpp"
#include "adv/SkipSpace.hpp"
#include "adv/ReadLuaExpr.hpp"
#include "adv/ReadStringLiteral.hpp"
#include "adv/ReadTable.hpp"
#include "adv/RecoverFromError.hpp"



/*

	TODO:

	[_] EOS handling

	[X] chunk ::= block

	[X] block ::= {stat} [retstat]

	[X] stat ::= [X] ‘;’ |
		[X] varlist ‘=’ explist |
		[X] functioncall |
		[X] label |
		[X] break |
		[X] goto Name |
		[X] do block end |
		[X] while exp do block end |
		[X] repeat block until exp |
		[X] if exp then block {elseif exp then block} [else block] end |
		[X] for Name ‘=’ exp ‘,’ exp [‘,’ exp] do block end |
		[X] for namelist in explist do block end |
		[X] function funcname funcbody |
		[X] local function Name funcbody |
		[X] local attnamelist [‘=’ explist]

	[X] attnamelist ::=  Name attrib {‘,’ Name attrib}

	[X] attrib ::= [‘<’ Name ‘>’]

	[X] retstat ::= return [explist] [‘;’]

	[X] label ::= ‘::’ Name ‘::’

	[X] funcname ::= Name {‘.’ Name} [‘:’ Name]

	[X] varlist ::= var {‘,’ var}

	[X] var ::=  Name | prefixexp ‘[’ exp ‘]’ | prefixexp ‘.’ Name

	[X] namelist ::= Name {‘,’ Name}

	[X] explist ::= exp {‘,’ exp}

	[X] exp ::=  [X]nil | [X]false | [X]true | [X]Numeral | [X]LiteralString | [X]‘...’ | [X]functiondef |
		 [X]prefixexp | [X]tableconstructor | [X]exp binop exp | [X]unop exp

	[X] prefixexp ::= var | functioncall | ‘(’ exp ‘)’

	[X] functioncall ::=  prefixexp args | prefixexp ‘:’ Name args

	[X] args ::=  ‘(’ [explist] ‘)’ | tableconstructor | LiteralString

	[X] functiondef ::= function funcbody

	[X] funcbody ::= ‘(’ [parlist] ‘)’ block end

	[X] parlist ::= namelist [‘,’ ‘...’] | ‘...’

	[X] tableconstructor ::= ‘{’ [fieldlist] ‘}’

	[X] fieldlist ::= field {fieldsep field} [fieldsep]

	[X] field ::= ‘[’ exp ‘]’ ‘=’ exp | Name ‘=’ exp | exp

	[X] fieldsep ::= ‘,’ | ‘;’

	[X] binop ::=  ‘+’ | ‘-’ | ‘*’ | ‘/’ | ‘//’ | ‘^’ | ‘%’ |
		 ‘&’ | ‘~’ | ‘|’ | ‘>>’ | ‘<<’ | ‘..’ |
		 ‘<’ | ‘<=’ | ‘>’ | ‘>=’ | ‘==’ | ‘~=’ |
		 and | or

	[X] unop ::= ‘-’ | not | ‘#’ | ‘~’

*/


namespace sluaParse
{
	//Doesnt skip space, the current character must be a valid args starter
	inline Args readArgs(AnyInput auto& in, const bool allowVarArg)
	{
		const char ch = in.peek();
		if (ch == '"' || ch=='\'' || ch=='[')
		{
			return ArgsType::LITERAL(readStringLiteral(in, ch));
		}
		else if (ch == '(')
		{
			in.skip();//skip start
			skipSpace(in);
			ArgsType::EXPLIST res{};
			if (in.peek() == ')')// Check if 0 args
			{
				in.skip();
				return res;
			}
			res.v = readExpList(in,allowVarArg);
			requireToken(in, ")");
			return res;
		}
		else if (ch == '{')
		{
			return ArgsType::TABLE(readTableConstructor(in,allowVarArg));
		}
		throw UnexpectedCharacterError(std::format(
			"Expected function arguments ("
			LUACC_SINGLE_STRING(",")
			" or "
			LUACC_SINGLE_STRING(";")
			"), found " LUACC_SINGLE_STRING("{}")
			"{}"
		, ch, errorLocStr(in)));
	}

	//startCh == in.peek() !!!
	inline bool isBasicBlockEnding(AnyInput auto& in, const char startCh)
	{
		if (startCh == 'u')
		{
			if (checkTextToken(in, "until"))
				return true;
		}
		else if (startCh == 'e')
		{
			const char ch1 = in.peekAt(1);
			if (ch1 == 'n')
			{
				if (checkTextToken(in, "end"))
					return true;
			}
			else if (ch1 == 'l')
			{
				if (checkTextToken(in, "else") || checkTextToken(in, "elseif"))
					return true;
			}
		}
		return false;
	}

	template<bool isLoop>
	inline Block readBlock(AnyInput auto& in,const bool allowVarArg)
	{
		/*
			block ::= {stat} [retstat]
			retstat ::= return [explist] [‘;’]
		*/

		skipSpace(in);

		Block ret{};
		ret.start = in.getLoc();

		while (true)
		{
			skipSpace(in);

			if (!in)//File ended, so block ended too
				break;

			const char ch = in.peek();

			if (ch == 'r')
			{
				if (checkReadTextToken(in, "return"))
				{
					ret.hadReturn = true;

					skipSpace(in);

					const char ch1 = in.peek();

					if (ch1 == ';')
						in.skip();//thats it
					else if (!isBasicBlockEnding(in, ch1))
					{
						ret.retExprs = readExpList(in, allowVarArg);
						readOptToken(in, ";");
					}
					break;// no more loop
				}
			}
			else if (isBasicBlockEnding(in, ch))
				break;// no more loop

			// Not some end / return keyword, must be a statement

			ret.statList.emplace_back(readStatment<isLoop>(in, allowVarArg));
		}
		ret.end = in.getLoc();
		return ret;
	}

	//Pair{fn, hadErr}
	inline std::pair<Function,bool> readFuncBody(AnyInput auto& in)
	{
		/*
			funcbody ::= ‘(’ [parlist] ‘)’ block end
			parlist ::= namelist [‘,’ ‘...’] | ‘...’
		*/
		Function ret{};

		Position place = in.getLoc();

		requireToken(in, "(");

		skipSpace(in);

		const char ch = in.peek();

		if (ch == '.')
		{
			requireToken(in, "...");
			ret.hasVarArgParam = true;
		}
		else if (ch != ')')
		{//must have non-empty namelist
			ret.params.emplace_back(readName(in));

			while (checkReadToken(in, ","))
			{
				if (checkReadToken(in, "..."))
				{
					ret.hasVarArgParam = true;
					break;//cant have anything after the ... arg
				}
				ret.params.emplace_back(readName(in));
			}
		}

		requireToken(in, ")");
		try
		{
			ret.block = readBlock<false>(in, ret.hasVarArgParam);
		}
		catch (const ParseError& e)
		{
			in.handleError(e.m);

			if(recoverErrorTextToken(in,"end"))
				return { std::move(ret),true };// Found it, recovered!

			//End of stream, and no found end's, maybe the error is a missing "end"?
			throw FailedRecoveryError(std::format(
				"Missing " LUACC_SINGLE_STRING("end") ", maybe for " LC_function " at {} ?",
				errorLocStr(in, place)
			));
		}
		requireToken(in, "end");

		return { std::move(ret),false };
	}

	inline std::string readLabel(AnyInput auto& in)
	{
		//label ::= ‘::’ Name ‘::’

		requireToken(in, "::");

		const std::string res = readName(in);

		requireToken(in, "::");

		return res;
	}

	template<bool isLoop>
	inline Block readDoEndBlock(AnyInput auto& in, const bool allowVarArg)
	{
		requireToken(in, "do");
		Block bl = readBlock<isLoop>(in,allowVarArg);
		requireToken(in, "end");

		return bl;
	}

	template<bool isLoop>
	inline Statement readStatment(AnyInput auto& in,const bool allowVarArg)
	{
		/*
		 varlist ‘=’ explist |
		 functioncall |
		*/

		skipSpace(in);

		Statement ret;
		ret.place = in.getLoc();

		const char firstChar = in.peek();
		switch (firstChar)
		{
		case ';':
			in.skip();
			ret.data = StatementType::SEMICOLON();
			return ret;

		case ':'://must be label
			ret.data = StatementType::LABEL(readLabel(in));
			return ret;

		case 'f'://for?,func?
			if (checkReadTextToken(in, "for"))
			{
				/*
				 for Name ‘=’ exp ‘,’ exp [‘,’ exp] do block end |
				 for namelist in explist do block end |
				*/

				NameList names = readNameList(in);

				if (names.size() == 1 && checkReadToken(in, "="))//1 name, then MAYBE equal
				{
					StatementType::FOR_LOOP_NUMERIC res{};
					res.varName = names[0];

					// for Name ‘=’ exp ‘,’ exp [‘,’ exp] do block end | 
					res.start = readExpr(in,allowVarArg);
					requireToken(in, ",");
					res.end = readExpr(in,allowVarArg);

					if (checkReadToken(in, ","))
						res.step = readExpr(in,allowVarArg);

					res.bl = readDoEndBlock<true>(in,allowVarArg);

					ret.data = std::move(res);
					return ret;
				}
				// Generic Loop
				// for namelist in explist do block end | 

				StatementType::FOR_LOOP_GENERIC res{};
				res.varNames = names;

				requireToken(in, "in");
				res.exprs = readExpList(in,allowVarArg);
				res.bl = readDoEndBlock<true>(in,allowVarArg);

				ret.data = std::move(res);
				return ret;
			}
			if (checkReadTextToken(in, "function"))
			{ // function funcname funcbody
				StatementType::FUNCTION_DEF res{};

				res.place = in.getLoc();

				res.name = readFuncName(in);

				try
				{
					auto [fun, err] = readFuncBody(in);
					res.func = std::move(fun);
					if(err)
					{
						in.handleError(std::format(
							"In " LC_function " " LUACC_SINGLE_STRING("{}") " at {}",
							res.name, errorLocStr(in, res.place)
						));
					}
				}
				catch (const ParseError& e)
				{
					in.handleError(e.m);
					throw ErrorWhileContext(std::format(
						"In " LC_function " " LUACC_SINGLE_STRING("{}") " at {}",
						res.name, errorLocStr(in, res.place)
					));
				}

				ret.data = std::move(res);
				return ret;
			}
			break;
		case 'l'://local?
			if (checkReadTextToken(in, "local"))
			{//func, or var
				/*
					local function Name funcbody |
					local attnamelist [‘=’ explist]
				*/
				if (checkReadTextToken(in, "function"))
				{ // local function Name funcbody
					StatementType::LOCAL_FUNCTION_DEF res;

					res.place = in.getLoc();

					res.name = readFuncName(in);

					try
					{
						auto [fun, err] = readFuncBody(in);
						res.func = std::move(fun);
						if (err)
						{

							in.handleError(std::format(
								"In " LC_function " " LUACC_SINGLE_STRING("{}") " at {}",
								res.name, errorLocStr(in, res.place)
							));
						}
					}
					catch (const ParseError& e)
					{
						in.handleError(e.m);
						throw ErrorWhileContext(std::format(
							"In " LC_function " " LUACC_SINGLE_STRING("{}") " at {}",
							res.name, errorLocStr(in, res.place)
						));
					}

					ret.data = std::move(res);
					return ret;
				}
				// Local Variable

				StatementType::LOCAL_ASSIGN res;
				res.names = readAttNameList(in);

				if (checkReadToken(in, "="))
				{// [‘=’ explist]
					res.exprs = readExpList(in,allowVarArg);
				}
				ret.data = std::move(res);
				return ret;
			}
			break;
		case 'd'://do?
			if (checkReadTextToken(in, "do")) // do block end
			{
				Block bl = readBlock<isLoop>(in,allowVarArg);
				requireToken(in, "end");
				ret.data = StatementType::DO_BLOCK(std::move(bl));
				return ret;
			}
			break;
		case 'b'://break?
			if (checkReadTextToken(in, "break"))
			{
				if constexpr (!isLoop)
				{
					throw ReservedNameError(std::format(
						"Break used outside of loop{}"
						, errorLocStr(in)));
				}
				ret.data = StatementType::BREAK();
				return ret;
			}
			break;
		case 'g'://goto?
			if (checkReadTextToken(in, "goto"))//goto Name
			{
				ret.data = StatementType::GOTO(readName(in));
				return ret;
			}
			break;
		case 'w'://while?
			if (checkReadTextToken(in, "while"))
			{ // while exp do block end
				Expression expr = readExpr(in,allowVarArg);
				Block bl = readDoEndBlock<true>(in,allowVarArg);
				ret.data = StatementType::WHILE_LOOP(std::move(expr), std::move(bl));
				return ret;
			}
			break;
		case 'r'://repeat?
			if (checkReadTextToken(in, "repeat"))
			{ // repeat block until exp
				Block bl = readBlock<true>(in,allowVarArg);
				requireToken(in, "until");
				Expression expr = readExpr(in,allowVarArg);

				ret.data = StatementType::REPEAT_UNTIL({ std::move(expr), std::move(bl) });
				return ret;
			}
			break;
		case 'i'://if?
			if (checkReadTextToken(in, "if"))
			{ // if exp then block {elseif exp then block} [else block] end

				StatementType::IF_THEN_ELSE res{};

				res.cond = readExpr(in,allowVarArg);

				requireToken(in, "then");

				res.bl = readBlock<isLoop>(in,allowVarArg);

				while (checkReadTextToken(in, "elseif"))
				{
					Expression elExpr = readExpr(in,allowVarArg);
					requireToken(in, "then");
					Block elBlock = readBlock<isLoop>(in,allowVarArg);

					res.elseIfs.emplace_back( std::move(elExpr),std::move(elBlock));
				}

				if (checkReadTextToken(in, "else"))
					res.elseBlock = readBlock<isLoop>(in,allowVarArg);

				requireToken(in, "end");

				ret.data = std::move(res);
				return ret;
			}
			break;

		default://none of the above...
			break;
		}

		ret.data = parsePrefixExprVar<StatementData,false>(in,allowVarArg, firstChar);
		return ret;
	}

	struct ParsedFile
	{
		//TypeList types
		Block code;
	};
	/**
	 * @throws sluaParse::ParseFailError
	 */
	inline ParsedFile parseFile(AnyInput auto& in)
	{
		try
		{
			Block bl = readBlock<false>(in, true);

			if (in.hasError())
			{// Skip eof, as one of the errors might have caused that.
				throw ParseFailError();
			}

			skipSpace(in);
			if (in)
			{
				throw UnexpectedCharacterError(std::format(
					"Expected end of stream"
					", found " LUACC_SINGLE_STRING("{}")
					"{}"
					, in.peek(), errorLocStr(in)));
			}
			return { std::move(bl) };
		}
		catch (const BasicParseError& e)
		{
			in.handleError(e.m);
			throw ParseFailError();
		}
	}
}