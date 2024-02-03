#include "Parser.h"

#include <cstring>
#include <cmath>

#include "Enums.h"
#include "Errors/ParseError.hpp"

using namespace BSPStructs;
using namespace BSPEnums;
using BSPErrors::ParseError;

void BSPParser::GetLumpPtr(
	const uint8_t* pData, const size_t size,
	const Header* pHeader, const LUMP lumpType,
	const uint8_t** pPtrOut
)
{
	if (pData == nullptr || pHeader == nullptr || pPtrOut == nullptr) {
		throw ParseError("Data, header or output pointers are null", lumpType);
	}

	const Lump& lump = pHeader->lumps[static_cast<size_t>(lumpType)];
	if (lump.offset <= 0) {
		throw ParseError("Lump offset is less than 1", lumpType);
	}
	if (lump.offset + lump.length > size) {
		throw ParseError("Lump offset plus length overruns the data", lumpType);
	}

	*pPtrOut = pData + lump.offset;
}

void BSPParser::ParseHeader(
	const uint8_t* pData, const size_t size,
	const Header** pHeaderPtr
)
{
	if (pData == nullptr || pHeaderPtr == nullptr) {
		throw ParseError("Data or header pointers are null", LUMP::NONE);
	}
	if (size < sizeof(Header)) {
		throw ParseError("Data size is smaller than the file header", LUMP::NONE);
	}

	*pHeaderPtr = reinterpret_cast<const Header*>(pData);
	if ((*pHeaderPtr)->ident != IDBSPHEADER) {
		throw ParseError("Header's identifier is not 'VBSP'", LUMP::NONE);
	}
}

template<class LumpDatatype>
void ParseLumpBase(
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
	) {
		throw ParseError("Data, header, array or length pointers are null", lump);
	}

	if (pHeader->lumps[static_cast<size_t>(lump)].length % sizeof(LumpDatatype) != 0) {
		throw ParseError("Size of the lump is not an exact multiple of the size of its items", lump);
	}

	const uint8_t* pLumpData;
	BSPParser::GetLumpPtr(pData, size, pHeader, lump, &pLumpData);

	*pLength = pHeader->lumps[static_cast<size_t>(lump)].length / sizeof(LumpDatatype);
	if (*pLength > max) {
		throw ParseError("Number of lump items exceeds the Source engine maximum", lump);
	}

	*pArray = reinterpret_cast<const LumpDatatype*>(pLumpData);
}

void BSPParser::ParseArray(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const char** pArray, size_t* pLength,
	const LUMP lump, const size_t max
)
{
	ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		lump, max
	);
}

void BSPParser::ParseArray(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const int32_t** pArray, size_t* pLength,
	const LUMP lump, const size_t max
)
{
	ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		lump, max
	);
}

void BSPParser::ParseArray(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const Vector** pArray, size_t* pLength,
	const LUMP lump, const size_t max
)
{
	ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		lump, max
	);
}

void BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const Plane** pArray, size_t* pLength
) {
	ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::PLANES, MAX_MAP_PLANES
	);
}

void BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const Edge** pArray, size_t* pLength
) {
	ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::EDGES, MAX_MAP_EDGES
	);
}

void BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const Face** pArray, size_t* pLength
) {
	ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::FACES, MAX_MAP_FACES
	);
}

void BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const TexInfo** pArray, size_t* pLength
) {
	ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::TEXINFO, MAX_MAP_TEXINFO
	);
}

void BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const TexData** pArray, size_t* pLength
) {
	ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::TEXDATA, MAX_MAP_TEXDATA
	);
}

void BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const Model** pArray, size_t* pLength
) {
	ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::MODELS, MAX_MAP_MODELS
	);
}

void BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const DispInfo** pArray, size_t* pLength
) {
	ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::DISPINFO, MAX_MAP_DISPINFO
	);
}

void BSPParser::ParseLump(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const DispVert** pArray, size_t* pLength
) {
	ParseLumpBase(
		pData, size,
		pHeader,
		pArray, pLength,
		LUMP::DISP_VERTS, MAX_MAP_DISP_VERTS
	);
}
