#include "Parser.h"

#include <cstring>
#include <cmath>

#include "Enums.h"

using namespace BSPStructs;
using namespace BSPEnums;

bool BSPParser::GetLumpPtr(
	const uint8_t* pData, const size_t size,
	const Header* pHeader, const LUMP lumpType,
	const uint8_t** pPtrOut
)
{
	if (pData == nullptr || pHeader == nullptr || pPtrOut == nullptr) return false;

	const Lump& lump = pHeader->lumps[static_cast<size_t>(lumpType)];
	if (lump.offset <= 0) return false;
	if (lump.offset + lump.length >= size) return false;

	*pPtrOut = pData + lump.offset;
	return true;
}

bool BSPParser::ParseHeader(
	const uint8_t* pData, const size_t size,
	const Header** pHeaderPtr
)
{
	if (pData == nullptr || pHeaderPtr == nullptr) return false;
	if (size < sizeof(Header)) return false;

	*pHeaderPtr = reinterpret_cast<const Header*>(pData);
	return (*pHeaderPtr)->ident == IDBSPHEADER;
}

template<class LumpDatatype>
bool ParseLumpBase(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const LumpDatatype** pArray, size_t* pLength,
	const LUMP lump, const size_t max
)
{
	if (
		pData == nullptr ||
		pHeader == nullptr ||
		pArray == nullptr ||
		pLength == nullptr
	) return false;

	if (pHeader->lumps[static_cast<size_t>(lump)].length % sizeof(LumpDatatype) != 0)
		return false;

	const uint8_t* pLumpData;
	if (BSPParser::GetLumpPtr(pData, size, pHeader, lump, &pLumpData) == false)
		return false;

	*pLength = pHeader->lumps[static_cast<size_t>(lump)].length / sizeof(LumpDatatype);
	if (*pLength > max) return false;

	*pArray = reinterpret_cast<const LumpDatatype*>(pLumpData);
	return true;
}

bool BSPParser::ParseArray(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const char** pArray, size_t* pLength,
	const LUMP lump, const size_t max
)
{
	return ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		lump, max
	);
}

bool BSPParser::ParseArray(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const int32_t** pArray, size_t* pLength,
	const LUMP lump, const size_t max
)
{
	return ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		lump, max
	);
}

bool BSPParser::ParseArray(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const Vector** pArray, size_t* pLength,
	const LUMP lump, const size_t max
)
{
	return ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		lump, max
	);
}

bool BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const Plane** pArray, size_t* pLength
) {
	return ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::PLANES, MAX_MAP_PLANES
	);
}

bool BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const Edge** pArray, size_t* pLength
) {
	return ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::EDGES, MAX_MAP_EDGES
	);
}

bool BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const Face** pArray, size_t* pLength
) {
	return ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::FACES, MAX_MAP_FACES
	);
}

bool BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const TexInfo** pArray, size_t* pLength
) {
	return ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::TEXINFO, MAX_MAP_TEXINFO
	);
}

bool BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const TexData** pArray, size_t* pLength
) {
	return ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::TEXDATA, MAX_MAP_TEXDATA
	);
}

bool BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const Model** pArray, size_t* pLength
) {
	return ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::MODELS, MAX_MAP_MODELS
	);
}

bool BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const DispInfo** pArray, size_t* pLength
) {
	return ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::DISPINFO, MAX_MAP_DISPINFO
	);
}

bool BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const DispVert** pArray, size_t* pLength
) {
	return ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::DISP_VERTS, MAX_MAP_DISP_VERTS
	);
}
