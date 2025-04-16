/*
** See Copyright Notice inside Include.hpp
*/
#pragma once

#include <string>
#include <unordered_map>
#include <variant>

#include <slua/parser/State.hpp>
#include <slua/lang/BasicState.hpp>

namespace slua::mlvl
{
	using slua::lang::ModPath;
	using slua::lang::ExportData;

	/*
	
	Mid level plans:

	-----------------
	safety checker
	referenced name checker
	?stability checker?
	-----------------
	type inference?
	-----------------
	type checker?
	-----------------
	borrow checker?
	purity checker & inference?
	-----------------
	*/

	struct ModPathId
	{
		size_t val;
	};
	struct MpItmId
	{
		ModPathId mp;
		size_t valId;
	};
	struct TypeId : MpItmId
	{};
	struct ObjId : MpItmId
	{};
	struct LocalObjId
	{
		size_t valId;
	};



	struct BasicData
	{
		parse::Position defPos;
		ExportData exData;
	};




	namespace ObjType
	{
		struct FuncId :MpItmId {};
		struct VarId :MpItmId {};
		struct TypeId :MpItmId {};
		struct TraitId :MpItmId {};
		struct ImplId :MpItmId {};
		struct UseId :MpItmId {};
		struct MacroId :MpItmId {};
	}
	using Obj = std::variant<
		ObjType::FuncId,
		ObjType::VarId,
		ObjType::TypeId,
		ObjType::TraitId,
		ObjType::ImplId,
		ObjType::UseId,
		ObjType::MacroId
	>;

	namespace LocalObjType
	{
		struct FuncId :LocalObjId {};
		struct VarId :LocalObjId {};
		struct TypeId :LocalObjId {};
		struct TraitId :LocalObjId {};
		struct ImplId :LocalObjId {};
		struct UseId :LocalObjId {};
		struct MacroId :LocalObjId {};
	}
	using LocalObj = std::variant<
		LocalObjType::FuncId,
		LocalObjType::VarId,
		LocalObjType::TypeId,
		LocalObjType::TraitId,
		LocalObjType::ImplId,
		LocalObjType::UseId,
		LocalObjType::MacroId
	>;

	struct Func : BasicData
	{
		std::string name;
		parse::FunctionV<true> data;//todo: new struct for: flattened non-block syntax blocks, ref by "ptr"
	};
	struct Var : BasicData
	{
		// TODO: destructuring
		parse::ExpListV<true> vals;//TODO: same as func
	};
	struct Type : BasicData
	{
		std::string name;
		parse::ErrType val;
	};
	struct Trait : BasicData
	{
		std::string name;
		//TODO
	};
	struct Impl : BasicData
	{
		//TODO
	};
	struct Use : BasicData
	{
		//TODO: make a local copy of use variant, with a better data format, with support for id ptrs
		ModPathId base;
		parse::UseVariant value;
	};
	struct Macro : BasicData
	{
		std::string name;
		//TODO
	};

	struct MpData : BasicData
	{
		ModPath path;

		std::vector<Func> funcs;
		std::vector<Var> vars;
		std::vector<Type> types;
		std::vector<Trait> traits;
		std::vector<Impl> impl;
		std::vector<Use> uses;
		std::vector<Macro> macros;

		std::vector<LocalObj> objs;//MpData::_[...]

		std::vector<ModPathId> subModules;
		bool isInline : 1 = true;
	};
	struct CrateData
	{
		std::string name;
		ModPathId root;
	};
	struct MidState
	{
		std::vector<MpData> modPaths;
		std::vector<CrateData> crates;
	};
}