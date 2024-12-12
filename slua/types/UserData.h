/*
** See Copyright Notice inside Include.hpp
*/
#pragma once

#include <string>
#include <array>

namespace slua
{
	inline const size_t TYPE_ID_SIZE = 2;

// After all typeids are initialized, this will contain a value that will always be more than any existing type id
	inline uint16_t TYPE_ID_COUNT=0;

	template<typename TYPE>
	inline const uint16_t _typeId = TYPE_ID_COUNT++;

	template<typename TYPE>
	inline uint16_t getTypeId() {
		return _typeId<TYPE>;
	}
	// Changes the last 2 bytes into the types id
	// Uses sizeof(PTR_T) to find where to change stuff
	template<typename TYPE,typename PTR_T>
	inline void setTypeIdSeperate(PTR_T* ptr)
	{
		uint8_t* dataPtr = reinterpret_cast<uint8_t*>(ptr);

		const uint16_t typeId = getTypeId<TYPE>();

		dataPtr[sizeof(PTR_T)+0] = typeId;
		dataPtr[sizeof(PTR_T)+1] = typeId>>8;
	}
	// Changes the last 2 bytes into the types id
	// Uses sizeof(TYPE) to find where to change stuff
	template<typename TYPE>
	inline void setTypeId(TYPE* ptr) {
		setUserTypeIdSeperate<TYPE,TYPE>(ptr);
	}
}