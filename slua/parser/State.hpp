/*
** See Copyright Notice inside Include.hpp
*/
#pragma once

#include <string>
#include <span>
#include <vector>
#include <optional>
#include <memory>
#include <variant>

//https://www.lua.org/manual/5.4/manual.html
//https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form
//https://www.sciencedirect.com/topics/computer-science/backus-naur-form

#include "Enums.hpp"

namespace sluaParse
{
	struct Position
	{
		size_t line;
		size_t index;
	};

	struct Scope
	{
		std::vector<struct Variable> variables;
		std::vector<struct Function> functions;

		//Scope* owner;//nullptr -> this is the global scope

		Position start;
		Position end;
	};

	struct Variable
	{
		std::string name;
		//Scope* owner;//nullptr -> error

		Position place;
		//size_t typeId;
	};

	namespace FieldType { struct EXPR2EXPR; struct NAME2EXPR; struct EXPR; }

	using Field = std::variant<
		FieldType::EXPR2EXPR, // "'[' exp ']' = exp"
		FieldType::NAME2EXPR, // "Name = exp"
		FieldType::EXPR       // "exp"
	>;

	// �{� [fieldlist] �}�
	using TableConstructor = std::vector<Field>;



	using ExpList = std::vector<struct Expression>;

	struct Block
	{
		std::vector<struct Statement> statList;
		ExpList retExprs;//Special, may contain 0 elements (even with hadReturn)

		//Scope scope;

		Position start;
		Position end;

		bool hadReturn = false;
	};


	struct Parameter
	{
		std::string name;
		//size_t typeId;
	};

	struct Function
	{
		Position place;
		std::vector<Parameter> params;
		Block block;
		bool hasVarArgParam = false;// do params end with '...'
	};

	namespace ExprType
	{
		struct NIL {};													// "nil"
		struct FALSE {};												// "false"
		struct TRUE {};													// "true"
		struct NUMERAL { double v; };									// "Numeral"
		struct LITERAL_STRING { std::string v; };						// "LiteralString"
		struct VARARGS {};												// "..."
		struct FUNCTION_DEF { Function v; };							// "functiondef"
		struct PREFIX_EXP { std::unique_ptr<struct PrefixExpr> v; };	// "prefixexp"
		struct TABLE_CONSTRUCTOR { TableConstructor v; };				// "tableconstructor"

		struct MULTI_OPERATION
		{
			std::unique_ptr<struct Expression> first;
			std::vector<std::pair<BinOpType, struct Expression>> extra;//size>=1
		};      // "exp binop exp"

		//struct UNARY_OPERATION{UnOpType,std::unique_ptr<struct Expression>};     // "unop exp"	//Inlined as opt prefix


		struct NUMERAL_I64 { int64_t v; };            // "Numeral"
	}

	using ExprData = std::variant<
		ExprType::NIL,                  // "nil"
		ExprType::FALSE,                // "false"
		ExprType::TRUE,                 // "true"
		ExprType::NUMERAL,				// "Numeral" (e.g., a floating-point number)
		ExprType::NUMERAL_I64,			// "Numeral"
		//ExprType::NUMERAL_U64,		// "Numeral"
		ExprType::LITERAL_STRING,		// "LiteralString"
		ExprType::VARARGS,              // "..." (varargs)
		ExprType::FUNCTION_DEF,			// "functiondef"
		ExprType::PREFIX_EXP,			// "prefixexp"
		ExprType::TABLE_CONSTRUCTOR,	// "tableconstructor"

		ExprType::MULTI_OPERATION		// "exp binop exp {binop exp}"  // added {binop exp}, cuz multi-op

		//ExprType::UNARY_OPERATION,	// "unop exp"
	>;

	struct Expression
	{
		ExprData data;
		Position place;
		UnOpType unOp;//might be NONE
	};

	namespace FieldType
	{
		struct EXPR2EXPR { Expression i; Expression v; };	// "�[� exp �]� �=� exp"
		struct NAME2EXPR { std::string i; Expression v; };	// "Name �=� exp"
		struct EXPR { Expression v; };						// "exp"
	};

	namespace VarType
	{
		struct INDEX_STR { std::unique_ptr<struct PrefixExpr> var; std::string idx; };	// "prefixexp.Name"
		struct INDEX { std::unique_ptr<struct PrefixExpr> var; Expression idx; };		// "prefixexp [ exp ]"
		struct VAR_NAME { std::string var; };											// "Name"
	}

	using Var = std::variant<
		VarType::INDEX,
		VarType::INDEX_STR,
		VarType::VAR_NAME
	>;

	namespace PrefixExprType
	{
		struct VAR { Var v; };										// "var"
		struct FUNC_CALL { std::unique_ptr<struct FuncCall> c; };	// "functioncall"
		struct EXPR { Expression e; };								// "'(' exp ')'"
	}
	using PrefixExpr = std::variant<
		PrefixExprType::VAR,
		PrefixExprType::FUNC_CALL,
		PrefixExprType::EXPR
	>;

	namespace ArgsType
	{
		struct EXPLIST { ExpList v; };			// "'(' [explist] ')'"
		struct TABLE { TableConstructor v; };	// "tableconstructor"
		struct LITERAL { std::string v; };		// "LiteralString"
	};
	using Args = std::variant<
		ArgsType::EXPLIST,
		ArgsType::TABLE,
		ArgsType::LITERAL
	>;

	struct FuncCall
	{
		PrefixExpr val;
		std::optional<std::string> funcName;// for abc:___()
		Args args;
	};

	struct AttribName
	{
		std::string name;
		std::string attrib;//empty -> no attrib
	};

	using AttribNameList = std::vector<AttribName>;
	using NameList = std::vector<std::string>;

	namespace StatementType
	{
		struct NONE {}; //Here, so variant has a default value (DO NOT USE)


		struct SEMICOLON {};									// ";"
		struct ASSIGN { AttribNameList name; ExpList exprs; };  // "varlist = explist" //e.size must be > 0
		struct FUNC_CALL { FuncCall v; };						// "functioncall"
		struct LABEL {};										// "label"
		struct BREAK { std::string v; };						// "break"
		struct GOTO { std::string v; };							// "goto Name"
		struct DO_BLOCK { Block bl; };							// "do block end"
		struct WHILE_LOOP { Expression cond; Block bl; };       // "while exp do block end"
		struct REPEAT_UNTIL :WHILE_LOOP {};						// "repeat block until exp"

		// "if exp then block {elseif exp then block} [else block] end"
		struct IF_THEN_ELSE
		{
			Expression cond;
			Block bl;
			std::vector<std::pair<Expression, Block>> elseIfs;
			std::optional<Block> elseBlock;
		};
		// "for Name = exp , exp [, exp] do block end"
		struct FOR_LOOP_NUMERIC
		{
			std::string varName;
			Expression start;
			Expression end;//inclusive
			std::optional<Expression> step;
			Block bl;
		};
		// "for namelist in explist do block end"
		struct FOR_LOOP_GENERIC
		{
			NameList varNames;
			ExpList exprs;//size must be > 0
			Block bl;
		};
		struct FUNCTION_DEF { std::string name; Function func; };// "function funcname funcbody"    //n may contain dots, 1 colon
		struct LOCAL_FUNCTION_DEF :FUNCTION_DEF {};        // "local function Name funcbody" //n may not ^^^
		struct LOCAL_ASSIGN :ASSIGN {};			   // "local attnamelist [= explist]" //e.size 0 means "only define, no assign"
	};

	using StatementData = std::variant <
		StatementType::NONE,

		StatementType::SEMICOLON, // ";"

		StatementType::ASSIGN, // "varlist = explist"

		StatementType::LOCAL_ASSIGN, // "local attnamelist [= explist]"

		StatementType::FUNC_CALL, // "functioncall"
		StatementType::LABEL, // "label"
		StatementType::BREAK, // "break"
		StatementType::GOTO, // "goto Name"
		StatementType::DO_BLOCK, // "do block end"
		StatementType::WHILE_LOOP, // "while exp do block end"
		StatementType::REPEAT_UNTIL, // "repeat block until exp"

		StatementType::IF_THEN_ELSE, // "if exp then block {elseif exp then block} [else block] end"

		StatementType::FOR_LOOP_NUMERIC, // "for Name = exp , exp [, exp] do block end"

		StatementType::FOR_LOOP_GENERIC, // "for namelist in explist do block end"

		StatementType::FUNCTION_DEF, // "function funcname funcbody"

		StatementType::LOCAL_FUNCTION_DEF // "local function Name funcbody"
	> ;


	struct Statement
	{
		StatementData data;
		Position place;
	};
}