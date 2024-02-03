#pragma once

#include "Structs.h"
#include "Enums.h"

namespace BSPParser
{
	void GetLumpPtr(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader, BSPEnums::LUMP lump,
		const uint8_t** pPtrOut
	);

	void ParseHeader(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header** pHeaderPtr
	);

	void ParseArray(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader,
		const char** pArray, size_t* pLength,
		BSPEnums::LUMP lump, size_t max
	);

	void ParseArray(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader,
		const int32_t** pArray, size_t* pLength,
		BSPEnums::LUMP lump, size_t max
	);

	void ParseArray(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::Vector** pArray, size_t* pLength,
		BSPEnums::LUMP lump, size_t max
	);

	void ParseLump(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::Plane** pArray, size_t* pLength
	);


	void ParseLump(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::Edge** pArray, size_t* pLength
	);

	void ParseLump(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::Face** pArray, size_t* pLength
	);

	void ParseLump(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::TexInfo** pArray, size_t* pLength
	);

	void ParseLump(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::TexData** pArray, size_t* pLength
	);

	void ParseLump(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::Model** pArray, size_t* pLength
	);

	void ParseLump(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::DispInfo** pArray, size_t* pLength
	);

	void ParseLump(
		const uint8_t* pData, size_t size,
		const BSPStructs::Header* pHeader,
		const BSPStructs::DispVert** pArray, size_t* pLength
	);
}
