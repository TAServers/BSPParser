#include "Parser.h"

#include <cstring>

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

bool BSPParser::ParseHeader(const uint8_t* pData, const size_t size, const Header** pHeaderPtr)
{
	if (pData == nullptr || pHeaderPtr == nullptr) return false;
	if (size < sizeof(Header)) return false;

	*pHeaderPtr = reinterpret_cast<const Header*>(pData);
	return (*pHeaderPtr)->ident == IDBSPHEADER;
}

bool BSPParser::ParseFaces(
	const uint8_t* pData, const size_t size,
	const Header* pHeader,
	const Face* const** pFaceArray, size_t* pNumFaces
)
{
	if (
		pData == nullptr ||
		pHeader == nullptr ||
		pFaceArray == nullptr ||
		pNumFaces == nullptr
	) return false;

	const uint8_t* pFaceData;
	if (GetLumpPtr(pData, size, pHeader, LUMP::FACES, &pFaceData) == false)
		return false;

	if (pHeader->lumps[static_cast<size_t>(LUMP::FACES)].length / sizeof(Face) > MAX_MAP_FACES)
		return false;

	*pFaceArray = reinterpret_cast<const Face* const*>(pFaceData);
	return true;
}
