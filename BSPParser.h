#pragma once

#include <cstdint>

#include "FileFormat/Parser.h"

class BSPMap
{
private:
	bool mIsValid = false;

	uint8_t* mpData;
	size_t mDataSize;

	const BSPStructs::Header* mpHeader;

	const BSPStructs::Edge* mpEdges;
	size_t mNumEdges;

	const int32_t* mpSurfEdges;
	size_t mNumSurfEdges;

	const BSPStructs::Face* mpFaces;
	size_t mNumFaces;

	const BSPStructs::TexInfo* mpTexInfos;
	size_t mNumTexInfos;

	const BSPStructs::TexData* mpTexData;
	size_t mNumTexDatas;

	const int32_t* mpTexDataStringTable;
	size_t mNumTexDataStringTableEntries;

	const char* mpTexDataStringData;
	size_t mNumTexDataStringDatas;

	template<class LumpDatatype>
	bool ParseLump(const LumpDatatype** pArray, size_t* pLength)
	{
		return BSPParser::ParseLump(
			mpData, mDataSize,
			mpHeader,
			pArray, pLength
		);
	}

public:
	BSPMap(const uint8_t* pFileData, const size_t dataSize);
	~BSPMap();

	bool IsValid() const;

	const BSPStructs::Face* GetFace(size_t index) const;
};
