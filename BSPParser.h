#pragma once

#include <cstdint>
#include <stdexcept>

#include "FileFormat/Parser.h"
#include "Errors/ParseError.hpp"

struct BSPTexture
{
	BSPEnums::SURF flags = BSPEnums::SURF::NONE;
	BSPStructs::Vector reflectivity{0, 0, 0};
	const char* path = nullptr;
	int32_t width = 0, height = 0;
};

struct BSPStaticProp
{
	BSPStructs::Vector pos{};
	BSPStructs::QAngle ang{};
	const char* model = nullptr;
	int32_t skin = 0;
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

	const BSPStructs::GameLump* mpGameLumps;
	size_t mNumGameLumps = 0U;

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

	const BSPStructs::DetailObjectDict* mpDetailObjectDict;
	size_t mNumDetailObjectDictEntries = 0U;

	const BSPStructs::DetailObject* mpDetailObjects;
	size_t mNumDetailObjects = 0U;

	const BSPStructs::StaticPropDict* mpStaticPropDict;
	size_t mNumStaticPropDictEntries = 0U;

	const BSPStructs::StaticPropLeaf* mpStaticPropLeaves;
	size_t mNumStaticPropLeaves = 0U;

	uint16_t mStaticPropsVersion;
	const BSPStructs::StaticPropV4* mpStaticPropsV4;
	const BSPStructs::StaticPropV5* mpStaticPropsV5;
	const BSPStructs::StaticPropV6* mpStaticPropsV6;
	int32_t mNumStaticProps = 0U;

	// Triangulation

	// Whether to create CW tris or CCW tris
	bool mClockwise = true;

	size_t mNumTris = 0U;

	BSPStructs::Vector* mpPositions = nullptr;
	BSPStructs::Vector* mpNormals = nullptr, * mpTangents = nullptr, * mpBinormals = nullptr;
	float* mpUVs = nullptr, * mpAlphas = nullptr;
	int16_t* mpTexIndices = nullptr;

	template<class LumpDatatype>
	void ParseLump(const LumpDatatype** pArray, size_t* pLength)
	{
		BSPParser::ParseLump(
			mpData, mDataSize,
			mpHeader,
			pArray, pLength
		);
	}

	void ParseGameLumps();

	template<class StaticProp>
	void ParseStaticPropLump(const BSPStructs::GameLump& gameLump, const StaticProp** pPtrOut)
	{
		using BSPErrors::ParseError;
		using BSPEnums::LUMP;

		if (gameLump.offset < 0) {
			throw ParseError("Lump offset is before the start of the data", LUMP::GAME_LUMP);
		}
		if (gameLump.offset + gameLump.length > mDataSize) {
			throw ParseError("Lump offset plus length overruns the data", LUMP::GAME_LUMP);
		}

		// Initialise to 3 * int32_t, as this is the number of counts in the game lump
		size_t totalLumpSize = sizeof(int32_t) * 3;
		if (totalLumpSize > gameLump.length) {
			throw ParseError("Static prop game lump length is less than minimum", LUMP::GAME_LUMP);
		}

		// Get ptr to number of static prop dict entries
		// no idea why valve decided to put the counts for all of these before each segment of the sprop lump,
		// cause it's a pain to parse
		const uint8_t* pHeader = mpData + gameLump.offset;
		mNumStaticPropDictEntries = *reinterpret_cast<const int32_t*>(pHeader);
		totalLumpSize += mNumStaticPropDictEntries * sizeof(BSPStructs::StaticPropDict);
		if (mNumStaticPropDictEntries < 0 || totalLumpSize > gameLump.length) {
			mNumStaticPropDictEntries = 0;
			throw ParseError("Number of static prop dictionary entries is less than 0 or exceeds game lump length", LUMP::GAME_LUMP);
		}

		// Move ptr over the int and save offset ptr
		pHeader += sizeof(int32_t);
		mpStaticPropDict = reinterpret_cast<const BSPStructs::StaticPropDict*>(pHeader);

		// Move ptr over the static prop dictionary entries, and increase lump size by the size of the leaf lump
		pHeader += mNumStaticPropDictEntries * sizeof(BSPStructs::StaticPropDict);
		mNumStaticPropLeaves = *reinterpret_cast<const int32_t*>(pHeader);
		totalLumpSize += mNumStaticPropLeaves * sizeof(BSPStructs::StaticPropLeaf);
		if (mNumStaticPropLeaves < 0 || totalLumpSize > gameLump.length) {
			mNumStaticPropLeaves = 0;
			throw ParseError("Number of static prop leaves is less than 0 or exceeds game lump length", LUMP::GAME_LUMP);
		}

		// Move ptr and save
		pHeader += sizeof(int32_t);
		mpStaticPropLeaves = reinterpret_cast<const BSPStructs::StaticPropLeaf*>(pHeader);

		// Move ptr over static prop leaves and add static prop lump size
		pHeader += mNumStaticPropLeaves * sizeof(BSPStructs::StaticPropLeaf);
		mNumStaticProps = *reinterpret_cast<const int32_t*>(pHeader);
		totalLumpSize += mNumStaticProps * sizeof(StaticProp);
		if (mNumStaticProps < 0) {
			mNumStaticProps = 0;
			throw ParseError("Number of static props is less than 0", LUMP::GAME_LUMP);
		}
		if (totalLumpSize != gameLump.length) {
			mNumStaticProps = 0;
			throw ParseError("Amount of parsed static prop data does not match the length of the game lump", LUMP::GAME_LUMP);
		}

		pHeader += sizeof(int32_t);
		*pPtrOut = reinterpret_cast<const StaticProp*>(pHeader);
	}

	template<class StaticProp>
	BSPStaticProp GetStaticPropInternal(const int32_t index, const StaticProp* pStaticProps) const
	{
		if (index < 0 || index >= mNumStaticProps)
			throw std::out_of_range("Static prop index out of bounds");

		uint16_t dictIdx = pStaticProps[index].propType;
		if (dictIdx >= mNumStaticPropDictEntries)
			throw std::out_of_range("Static prop dictionary index out of bounds");

		BSPStaticProp prop;
		prop.pos = pStaticProps[index].origin;
		prop.ang = pStaticProps[index].angles;
		prop.model = mpStaticPropDict[dictIdx].modelName;
		prop.skin = pStaticProps[index].skin;

		return prop;
	}

	bool IsFaceNodraw(const BSPStructs::Face* pFace) const;

	void FreeAll();

	bool CalcUVs(
		int16_t texInfoIdx,
		const BSPStructs::Vector* pos,
		float* pUVs
	) const;

	bool GetSurfEdgeVerts(int32_t index, BSPStructs::Vector* pVertA, BSPStructs::Vector* pVertB = nullptr) const;

	void Triangulate();

public:
	std::string errorReason = "Unknown error";
	BSPEnums::LUMP errorLump = BSPEnums::LUMP::NONE;

	// Parses and triangulates a BSP from raw data
	// clockwise sets which winding the triangles should have (default true)
	BSPMap(const uint8_t* pFileData, size_t dataSize, bool clockwise = true);
	~BSPMap();

	// Returns whether the BSP was loaded correctly
	[[nodiscard]] bool IsValid() const;

	// Gets th enumber of textures defined in the map
	[[nodiscard]] size_t GetNumTextures() const;

	// Returns relevant texture information for an index in the TexInfo lump
	[[nodiscard]] BSPTexture GetTexture(int16_t index) const;

	// Gets the number of triangles in the triangulated BSP data
    [[nodiscard]] size_t GetNumTris() const;

	// Gets the number of vertices in the triangulated BSP data
	[[nodiscard]] size_t GetNumVertices() const;

	// Returns a const pointer to the vertex positions as Vector structs (castable to floats)
    [[nodiscard]] const BSPStructs::Vector* GetVertices() const;

	// Returns a const pointer to the vertex normals as Vector structs (castable to floats)
    [[nodiscard]] const BSPStructs::Vector* GetNormals() const;

	// Returns a const pointer to the vertex tangents as Vector structs (castable to floats)
    [[nodiscard]] const BSPStructs::Vector* GetTangents() const;

	// Returns a const pointer to the vertex binormals as Vector structs (castable to floats)
    [[nodiscard]] const BSPStructs::Vector* GetBinormals() const;

	// Returns a const pointer to the vertex UVs as raw float data
    [[nodiscard]] const float* GetUVs() const;

	// Returns a const pointer to the vertex alphas as floats
    [[nodiscard]] const float* GetAlphas() const;

	// Returns a const pointer to the triangle TexInfo indices as an array of int16_t
    [[nodiscard]] const int16_t* GetTriTextures() const;

    [[nodiscard]] int32_t GetNumStaticProps() const;
    [[nodiscard]] BSPStaticProp GetStaticProp(int32_t index) const;
};
