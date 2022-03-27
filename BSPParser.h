#pragma once

#include <cstdint>

#include "FileFormat/Parser.h"

struct BSPTexture
{
	int32_t flags;
	BSPStructs::Vector reflectivity;
	const char* texturePath;
	int32_t width, height;
};

class BSPMap
{
private:
	bool mIsValid = false;

	uint8_t* mpData;
	size_t mDataSize;

	const BSPStructs::Header* mpHeader;

	const BSPStructs::Vector* mpVertices;
	size_t mNumVertices;

	const BSPStructs::Plane* mpPlanes;
	size_t mNumPlanes;

	const BSPStructs::Edge* mpEdges;
	size_t mNumEdges;

	const int32_t* mpSurfEdges;
	size_t mNumSurfEdges;

	const BSPStructs::Face* mpFaces;
	size_t mNumFaces;

	const BSPStructs::TexInfo* mpTexInfos;
	size_t mNumTexInfos;

	const BSPStructs::TexData* mpTexDatas;
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

	bool IsFaceNodraw(const BSPStructs::Face* pFace) const;

	void CalcUVs(
		const BSPStructs::TexInfo* pTexInfo, const float* pos,
		float* pUVs
	) const;

public:
	BSPMap(const uint8_t* pFileData, const size_t dataSize);
	~BSPMap();

	bool IsValid() const;

	BSPTexture GetTexture(const uint32_t index) const;

	size_t Triangulate(
		float** pPositions,
		float** pNormals, float** pTangents, float** pBinormals,
		float** pUVs, uint32_t** pTexIndices
	) const;
};
