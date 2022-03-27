#pragma once

#include "Structs.h"
#include "Enums.h"

namespace BSPParser
{
	bool GetLumpPtr(
		const uint8_t* pData, const size_t size,
		const BSPStructs::Header* pHeader, const  BSPEnums::LUMP lump,
		const uint8_t** pPtrOut
	);

	bool ParseHeader(
		const uint8_t* pData, const size_t size,
		const BSPStructs::Header** pHeaderPtr
	);

	bool ParseArray(
		const uint8_t* pData, const size_t size,
		const BSPStructs::Header* pHeader,
		const char** pArray, size_t* pLength,
		const  BSPEnums::LUMP lump, const size_t max
	);

	bool ParseArray(
		const uint8_t* pData, const size_t size,
		const BSPStructs::Header* pHeader,
		const int32_t** pArray, size_t* pLength,
		const  BSPEnums::LUMP lump, const size_t max
	);

	bool ParseArray(
		const uint8_t* pData, const size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::Vector** pArray, size_t* pLength,
		const  BSPEnums::LUMP lump, const size_t max
	);

	bool ParseLump(
		const uint8_t* pData, const size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::Plane** pArray, size_t* pLength
	);


	bool ParseLump(
		const uint8_t* pData, const size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::Edge** pArray, size_t* pLength
	);

	bool ParseLump(
		const uint8_t* pData, const size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::Face** pArray, size_t* pLength
	);

	bool ParseLump(
		const uint8_t* pData, const size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::TexInfo** pArray, size_t* pLength
	);

	bool ParseLump(
		const uint8_t* pData, const size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::TexData** pArray, size_t* pLength
	);
}
