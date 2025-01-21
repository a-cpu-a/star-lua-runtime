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

#include "ReadName.hpp"
#include "ReadOperators.hpp"
#include "RequireToken.hpp"
#include "SkipSpace.hpp"



/*

	TODO:

	[X] chunk ::= block

	[~] block ::= {stat} [retstat]

	[~] stat ::= [X] �;� |
		[_] varlist �=� explist |
		[_] functioncall |
		[X] label |
		[X] break |
		[X] goto Name |
		[X] do block end |
		[X] while exp do block end |
		[X] repeat block until exp |
		[X] if exp then block {elseif exp then block} [else block] end |
		[~] for Name �=� exp �,� exp [�,� exp] do block end |
		[~] for namelist in explist do block end |
		[_] function funcname funcbody |
		[_] local function Name funcbody |
		[~] local attnamelist [�=� explist]

	[_] attnamelist ::=  Name attrib {�,� Name attrib}

	[_] attrib ::= [�<� Name �>�]

	[~] retstat ::= return [explist] [�;�]

	[X] label ::= �::� Name �::�

	[_] funcname ::= Name {�.� Name} [�:� Name]

	[_] varlist ::= var {�,� var}

	[_] var ::=  Name | prefixexp �[� exp �]� | prefixexp �.� Name

	[_] namelist ::= Name {�,� Name}

	[X] explist ::= exp {�,� exp}

	[_] exp ::=  (~)nil | (~)false | (~)true | Numeral | LiteralString | (~)�...� | functiondef |
		 prefixexp | tableconstructor | [X] exp binop exp | [X] unop exp

	[_] prefixexp ::= var | functioncall | �(� exp �)�

	[_] functioncall ::=  prefixexp args | prefixexp �:� Name args

	[_] args ::=  �(� [explist] �)� | tableconstructor | LiteralString

	[_] functiondef ::= function funcbody

	[_] funcbody ::= �(� [parlist] �)� block end

	[_] parlist ::= namelist [�,� �...�] | �...�

	[_] tableconstructor ::= �{� [fieldlist] �}�

	[_] fieldlist ::= field {fieldsep field} [fieldsep]

	[_] field ::= �[� exp �]� �=� exp | Name �=� exp | exp

	[X] fieldsep ::= �,� | �;�

	[X] binop ::=  �+� | �-� | �*� | �/� | �//� | �^� | �%� |
		 �&� | �~� | �|� | �>>� | �<<� | �..� |
		 �<� | �<=� | �>� | �>=� | �==� | �~=� |
		 and | or

	[X] unop ::= �-� | not | �#� | �~�

*/


namespace sluaParse
{
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
	inline Block readBlock(AnyInput auto& in)
	{
		/*
			block ::= {stat} [retstat]
			retstat ::= return [explist] [�;�]
		*/
		Block ret{};
		ret.start = in.getLoc();

		//TODO: implement
		//0+ stat

		if (checkReadTextToken(in, "return"))
		{
			ret.hadReturn = true;
			//TODO: check for reserved tokens, or ';', to allow for empty returns
			ret.retExprs = readExpList(in);
			readOptToken(in, ";");
		}

		ret.end = in.getLoc();
		return ret;
	}

	inline Expression readExpr(AnyInput auto& in)
	{
		/*
			nil | false | true | Numeral | LiteralString | �...� | functiondef
			| prefixexp | tableconstructor | exp binop exp | unop exp
		*/

		Expression res;

		res.unOp = readOptUnOp(in);

		skipSpace(in);

		switch (in.peek())
		{
		case 'n':
			if (checkReadTextToken(in, "nil"))
			{
				//the in args
				break;
			}
			break;
		case 'f':

			if (checkReadTextToken(in, "false")) { break; }
			if (checkReadTextToken(in, "function")) { break; }
			break;
		case 't':
			if (checkReadTextToken(in, "true")) { break; }
			break;
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
			//TODO: numeral

			break;
		case '"':
		case '\'':
		case '[':
			//TODO: literal?
			break;
		case '.':
			if (checkReadToken(in, "..."))
			{
				//the in args
				break;
			}
			break;
		case '(':
			//TODO: prefixexp
			break;
		case '{':
			//TODO: tableconstructor
			break;
		}

		//check bin op


		const BinOpType binOp = readOptBinOp(in);

		if (binOp == BinOpType::NONE)
			return res;
	}

	inline std::string readLabel(AnyInput auto& in)
	{
		//label ::= �::� Name �::�

		requireToken(in, "::");

		const std::string res = readName(in);

		requireToken(in, "::");

		return res;
	}

	inline Statement readStatment(AnyInput auto& in)
	{
		/*
		 stat ::=  �;� |
		 varlist �=� explist |
		 functioncall |
		*/

		skipSpace(in);

		switch (in.peek())
		{
		case ';':
			return { StatementType::SEMICOLON(),in.getLoc() };

		case ':'://must be label
			return { StatementType::LABEL(readLabel(in)),in.getLoc() };

		case 'f'://for?,func?
			if (checkReadTextToken(in, "for"))
			{
				/*
				 for Name �=� exp �,� exp [�,� exp] do block end |
				 for namelist in explist do block end |
				*/

				//TODO: namelist

				if (true && checkReadToken(in, "="))//1 name, then equal
				{
					// for Name �=� exp �,� exp [�,� exp] do block end | 
					Expression initExpr = readExpr(in);
					requireToken(in, ",");
					Expression lmitExpr = readExpr(in);
					if (checkReadToken(in, ","))
					{
						Expression stepExpr = readExpr(in);
					}
					//TODO: export data
				}
				else
				{
					// for namelist in explist do block end | 
					requireToken(in, "in");
					ExpList expList = readExpList(in);
					//TODO: export data
				}
				requireToken(in, "do");
				Block bl = readBlock(in);
				requireToken(in, "end");
				break;//TODO: replace with return
			}
			if (checkReadTextToken(in, "function"))
			{ // function funcname funcbody
				break;//TODO: replace with return
			}
			break;
		case 'l'://local?
			if (checkReadTextToken(in, "local"))
			{//func, or var
				/*
					local function Name funcbody |
					local attnamelist [�=� explist]
				*/
				if (checkReadTextToken(in, "function"))
				{ // local function Name funcbody
					break;//TODO: replace with return
				}
				// Local Variable

				StatementType::LOCAL_ASSIGN ret;

				//TODO: attnamelist

				if (checkReadToken(in, "="))
				{ // [�=� explist]
					ret.exprs = readExpList(in);
				}

				return { ret,in.getLoc() };
			}
			break;
		case 'd'://do?
			if (checkReadTextToken(in, "do")) // do block end
			{
				Block bl = readBlock(in);
				requireToken(in, "end");
				return { StatementType::DO_BLOCK(bl), in.getLoc() };
			}
			break;
		case 'b'://break?
			if (checkReadTextToken(in, "break"))
				return { StatementType::BREAK(),in.getLoc() };
			break;
		case 'g'://goto?
			if (checkReadTextToken(in, "goto"))//goto Name
				return { StatementType::GOTO(readName(in)),in.getLoc() };
			break;
		case 'w'://while?
			if (checkReadTextToken(in, "while"))
			{ // while exp do block end
				Expression expr = readExpr(in);
				requireToken(in, "do");
				Block bl = readBlock(in);
				requireToken(in, "end");
				return { StatementType::WHILE_LOOP(expr,bl),in.getLoc() };
			}
			break;
		case 'r'://repeat?
			if (checkReadTextToken(in, "repeat"))
			{ // repeat block until exp
				Block bl = readBlock(in);
				requireToken(in, "until");
				Expression expr = readExpr(in);

				return { StatementType::REPEAT_UNTIL(expr,bl),in.getLoc() };
			}
			break;
		case 'i'://if?
			if (checkReadTextToken(in, "if"))
			{ // if exp then block {elseif exp then block} [else block] end

				StatementType::IF_THEN_ELSE ret{};

				ret.cond = readExpr(in);

				requireToken(in, "then");

				ret.bl = readBlock(in);

				while (checkReadTextToken(in, "elseif"))
				{
					Expression elExpr = readExpr(in);
					requireToken(in, "then");
					Block elBlock = readBlock(in);

					ret.elseIfs.push_back({ elExpr,elBlock });
				}

				if (checkReadTextToken(in, "else"))
					ret.elseBlock = readBlock(in);

				requireToken(in, "end");
				return { ret,in.getLoc() };
			}
			break;

		default://none of the above...
			break;
		}
		//try assign or func-call
	}

	struct ParsedFile
	{
		//TypeList types
		Block code;
	};

	inline ParsedFile parseFile(AnyInput auto& in)
	{
		return { readBlock(in) };
	}
}