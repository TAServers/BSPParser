#pragma once

#include <cstdint>

#include "FileFormat/Parser.h"

struct BSPTexture
{
	BSPEnums::SURF flags;
	BSPStructs::Vector reflectivity;
	const char* path;
	int32_t width, height;
};

class BSPMap
{
private:
	bool mIsValid = false;

	// Raw data
	uint8_t* mpData;
	size_t mDataSize = 0U;

	// Raw BSP structs
	const BSPStructs::Header* mpHeader;

	const BSPStructs::Vector* mpVertices;
	size_t mNumVertices = 0U;

	const BSPStructs::Plane* mpPlanes;
	size_t mNumPlanes = 0U;

	const BSPStructs::Edge* mpEdges;
	size_t mNumEdges = 0U;

	const int32_t* mpSurfEdges;
	size_t mNumSurfEdges = 0U;

	const BSPStructs::Face* mpFaces;
	size_t mNumFaces = 0U;

	const BSPStructs::TexInfo* mpTexInfos;
	size_t mNumTexInfos = 0U;

	const BSPStructs::TexData* mpTexDatas;
	size_t mNumTexDatas = 0U;

	const int32_t* mpTexDataStringTable;
	size_t mNumTexDataStringTableEntries = 0U;

	const char* mpTexDataStringData;
	size_t mNumTexDataStringDatas = 0U;

	const BSPStructs::Model* mpModels;
	size_t mNumModels = 0U;

	const BSPStructs::DispInfo* mpDispInfos;
	size_t mNumDispInfos = 0U;

	const BSPStructs::DispVert* mpDispVerts;
	size_t mNumDispVerts = 0U;

	// Triangulation

	// Whether to create CW tris or CCW tris
	bool mClockwise = true;

	size_t mNumTris = 0U;

	BSPStructs::Vector* mpPositions = nullptr;
	BSPStructs::Vector* mpNormals = nullptr, * mpTangents = nullptr, * mpBinormals = nullptr;
	float* mpUVs = nullptr, * mpAlphas = nullptr;
	int16_t* mpTexIndices = nullptr;

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

	void FreeAll();

	bool CalcUVs(
		const int16_t texInfoIdx, const BSPStructs::Vector* pos,
		float* pUVs
	) const;

	bool GetSurfEdgeVerts(const int32_t index, BSPStructs::Vector* pVertA, BSPStructs::Vector* pVertB = nullptr) const;

	bool Triangulate();

public:
	// Parses and triangulates a BSP from raw data
	// clockwise sets which winding the triangles should have (default true)
	BSPMap(const uint8_t* pFileData, const size_t dataSize, const bool clockwise = true);
	~BSPMap();

	// Returns whether the BSP was loaded correctly
	bool IsValid() const;

	// Returns relevant texture information for an index in the TexInfo lump
	BSPTexture GetTexture(const int16_t index) const;

	// Gets the number of triangles in the triangulated BSP data
	size_t GetNumTris() const;

	// Returns a const pointer to the vertex positions as Vector structs (castable to floats)
	const BSPStructs::Vector* GetVertices() const;

	// Returns a const pointer to the vertex normals as Vector structs (castable to floats)
	const BSPStructs::Vector* GetNormals() const;

	// Returns a const pointer to the vertex tangents as Vector structs (castable to floats)
	const BSPStructs::Vector* GetTangents() const;

	// Returns a const pointer to the vertex binormals as Vector structs (castable to floats)
	const BSPStructs::Vector* GetBinormals() const;

	// Returns a const pointer to the vertex UVs as raw float data
	const float* GetUVs() const;

	// Returns a const pointer to the vertex alphas as floats
	const float* GetAlphas() const;

	// Returns a const pointer to the triangle TexInfo indices as an array of int16_t
	const int16_t* GetTriTextures() const;
};
