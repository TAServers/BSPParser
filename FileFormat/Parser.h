#pragma once

#include "Structs.h"

namespace BSPParser
{
	bool GetLumpPtr(
		const uint8_t* pData, const size_t size,
		const Header* pHeader, const LUMP lump,
		const uint8_t** pPtrOut
	);

	bool ParseHeader(const uint8_t* pData, BSPStructs::Header* pHeader);

	bool ParseFaces(
		const uint8_t* pData, const size_t size,
		const Header* pHeader,
		Face** pFacePtr, size_t* pNumFaces
	);
}
