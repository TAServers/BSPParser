#include "BSPParser.h"
#include "FileFormat/Structs.h"

#include <cstring>
#include <stdlib.h>

// DEBUG, REMOVE
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace BSPStructs;
using namespace BSPEnums;

bool BSPMap::IsFaceNodraw(const Face* pFace) const
{
	return (
		pFace->texInfo < 0 ||
		(
			mpTexInfos[pFace->texInfo].flags &
			static_cast<int32_t>(SURF::SKY2D | SURF::NODRAW | SURF::SKIP | SURF::HITBOX)
		) != 0
	);
}

void BSPMap::CalcUVs(
	const TexInfo* pTexInfo, const float* pos,
	float* pUVs
) const
{
	if (pTexInfo->texData < 0 || pTexInfo->texData >= mNumTexDatas)
		throw std::runtime_error("BSP contains invalid texdata offset");
	const TexData* pTexData = mpTexDatas + pTexInfo->texData;

	const float* s = pTexInfo->textureVecs[0];
	const float* t = pTexInfo->textureVecs[1];

	pUVs[0] = s[0] * pos[0] + s[1] * pos[1] + s[2] * pos[2] + s[3];
	pUVs[1] = t[0] * pos[0] + t[1] * pos[1] + t[2] * pos[2] + t[3];

	pUVs[0] /= static_cast<float>(pTexData->width);
	pUVs[1] /= static_cast<float>(pTexData->height);
}

BSPMap::BSPMap(
	const uint8_t* pFileData, const size_t dataSize
) : mDataSize(dataSize)
{
	if (pFileData == nullptr || dataSize == 0U) return;

	mpData = reinterpret_cast<uint8_t*>(malloc(dataSize));
	memcpy(mpData, pFileData, dataSize);

	if (
		!BSPParser::ParseHeader(mpData, dataSize, &mpHeader) ||
		mpHeader->version < 19 || mpHeader->version > 21 ||
		!BSPParser::ParseArray(
			mpData, dataSize,
			mpHeader,
			&mpVertices, &mNumVertices,
			LUMP::VERTICES, MAX_MAP_VERTS
		) ||
		!ParseLump(&mpPlanes, &mNumPlanes) ||
		!ParseLump(&mpEdges, &mNumEdges) ||
		!BSPParser::ParseArray(
			mpData, dataSize,
			mpHeader,
			&mpSurfEdges, &mNumSurfEdges,
			LUMP::SURFEDGES, MAX_MAP_SURFEDGES
		) ||
		!ParseLump(&mpFaces, &mNumFaces) ||
		!ParseLump(&mpTexInfos, &mNumTexInfos) ||
		!ParseLump(&mpTexDatas, &mNumTexDatas) ||
		!BSPParser::ParseArray(
			mpData, dataSize,
			mpHeader,
			&mpTexDataStringTable, &mNumTexDataStringTableEntries,
			LUMP::TEXDATA_STRING_TABLE, MAX_MAP_TEXDATA_STRING_TABLE
		) ||
		!BSPParser::ParseArray(
			mpData, dataSize,
			mpHeader,
			&mpTexDataStringData, &mNumTexDataStringDatas,
			LUMP::TEXDATA_STRING_DATA, MAX_MAP_TEXDATA_STRING_DATA
		)
	) {
		free(mpData);
		mpData = nullptr;
		mDataSize = 0U;
		return;
	}

	mIsValid = true;
}

BSPMap::~BSPMap()
{
	if (mpData != nullptr) free(mpData);
}

bool BSPMap::IsValid() const { return mIsValid; }

BSPTexture BSPMap::GetTexture(const uint32_t index) const
{
	if (index >= mNumTexInfos)
		throw std::runtime_error("Texture index out of bounds");

	const TexInfo* pTexInfo = mpTexInfos + index;

	if (pTexInfo->texData < 0 || pTexInfo->texData >= mNumTexDatas)
		throw std::runtime_error("BSP contains invalid texdata offset");
	const TexData* pTexData = mpTexDatas + pTexInfo->texData;

	if (
		pTexData->nameStringTableId < 0 ||
		pTexData->nameStringTableId >= mNumTexDataStringTableEntries
	) throw std::runtime_error("BSP contains invalid texdata string table offset");

	BSPTexture ret{};
	ret.flags = pTexInfo->flags;
	ret.reflectivity = pTexData->reflectivity;
	ret.texturePath = mpTexDataStringData + mpTexDataStringTable[pTexData->nameStringTableId];
	ret.width = pTexData->width;
	ret.height = pTexData->height;

	return ret;
}

size_t BSPMap::Triangulate(
	float** pPositions,
	float** pNormals, float** pTangents, float** pBinormals,
	float** pUVs, uint32_t** pTexIndices
) const
{
	// Calculate number of tris in the map
	size_t numTris = 0U;
	for (const Face* pFace = mpFaces; pFace < mpFaces + mNumFaces; pFace++) {
		if (IsFaceNodraw(pFace)) continue;
		numTris += pFace->numEdges - 2;
	}

	// malloc buffers
	*pPositions  = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * numTris));
	*pNormals    = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * numTris));
	*pTangents   = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * numTris));
	*pBinormals  = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * numTris));
	*pUVs        = reinterpret_cast<float*>(malloc(sizeof(float) * 2U * numTris));
	*pTexIndices = reinterpret_cast<uint32_t*>(malloc(sizeof(uint32_t) * numTris));

	// Read data into buffers
	size_t triIdx = 0U;
	for (const Face* pFace = mpFaces; pFace < mpFaces + mNumFaces; pFace++) {
		if (IsFaceNodraw(pFace) || pFace->numEdges < 3) continue;

		// Get ref to root vertex
		// Get normal
		// Get texture index

		// For each edge (ignoring first and last)
		//	Add vertices to positions
		//  Compute tangent/bitangent and add to arrays

		if (pFace->firstEdge < 0 || pFace->firstEdge >= mNumSurfEdges)
			throw std::runtime_error("BSP contains invalid surfedge offset");

		int32_t edgeIdx = mpSurfEdges[pFace->firstEdge];
		if (abs(edgeIdx) > mNumEdges)
			throw std::runtime_error("BSP contains invalid edge offset");

		int32_t vertIdx = mpEdges[abs(edgeIdx)].vertices[edgeIdx < 0 ? 1 : 0];
		if (vertIdx < 0 || vertIdx > mNumVertices)
			throw std::runtime_error("BSP contains invalid vertex offset");

		const float* root = reinterpret_cast<const float*>(mpVertices + vertIdx);

		int16_t planeIdx = pFace->planeNum;
		if (planeIdx < 0 || planeIdx > mNumPlanes)
			throw std::runtime_error("BSP contains invalid plane offset");

		Vector normal = mpPlanes[planeIdx].normal;
		if (pFace->side != 0) {
			normal.x = -normal.x;
			normal.y = -normal.y;
			normal.z = -normal.z;
		}
	}
}

int main()
{
	const char* path = "C:\\Users\\eds\\Documents\\Programming\\gmodmaps\\spot_test\\spot_test.bsp";

	size_t size = std::filesystem::file_size(path);
	char* data = reinterpret_cast<char*>(malloc(size));

	auto fs = std::ifstream(path, std::ifstream::binary);
	fs.read(data, size);

	auto bsp = BSPMap(reinterpret_cast<uint8_t*>(data), size);
	free(data);

	std::cout << bsp.IsValid() << std::endl;

	return 0;
}
