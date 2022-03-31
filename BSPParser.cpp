#include "BSPParser.h"
#include "FileFormat/Structs.h"

#include <cstring>
#include <stdlib.h>
#include <cmath>
#include <stdexcept>

using namespace BSPStructs;
using namespace BSPEnums;

inline void Orthogonalise(float* v, const float* basis)
{
	float dot = v[0] * basis[0] + v[1] * basis[1] + v[2] * basis[2];
	v[0] -= basis[0] * dot;
	v[1] -= basis[1] * dot;
	v[2] -= basis[2] * dot;

	float length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= length;
	v[1] /= length;
	v[2] /= length;
}

inline void NegCross(const float* a, const float* b, float* out)
{
	out[0] = a[2] * b[1] - a[1] * b[2];
	out[1] = a[0] * b[2] - a[2] * b[0];
	out[2] = a[1] * b[0] - a[0] * b[1];
}

void CalcTangentBinormal(
	const float* p0, const float* p1, const float* p2,
	const float* uv0, const float* uv1, const float* uv2,
	const float* n,	float* t, float* b
)
{
	float edge0[3] = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
	float edge1[3] = { p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2] };

	float dUV0[2] = { uv1[0] - uv0[0], uv1[1] - uv0[1] };
	float dUV1[2] = { uv2[0] - uv0[0], uv2[1] - uv0[1] };

	float f = 1.f / (dUV0[0] * dUV1[1] - dUV1[0] * dUV0[1]);

	t[0] = f * (dUV1[1] * edge0[0] - dUV0[1] * edge1[0]);
	t[1] = f * (dUV1[1] * edge0[1] - dUV0[1] * edge1[1]);
	t[2] = f * (dUV1[1] * edge0[2] - dUV0[1] * edge1[2]);

	Orthogonalise(t, n);
	NegCross(n, t, b);
}

bool BSPMap::IsFaceNodraw(const Face* pFace) const
{
	return (
		pFace->texInfo < 0 ||
		(
			mpTexInfos[pFace->texInfo].flags &
			static_cast<int32_t>(SURF::SKY2D | SURF::SKY | SURF::NODRAW | SURF::SKIP | SURF::HITBOX)
		) != 0
	);
}

void BSPMap::FreeAll()
{
	if (mpData       != nullptr) free(mpData);
	if (mpPositions  != nullptr) free(mpPositions);
	if (mpNormals    != nullptr) free(mpNormals);
	if (mpTangents   != nullptr) free(mpTangents);
	if (mpBinormals  != nullptr) free(mpBinormals);
	if (mpUVs        != nullptr) free(mpUVs);
	if (mpTexIndices != nullptr) free(mpTexIndices);
}

bool BSPMap::CalcUVs(
	const int32_t texInfoIdx, const float* pos,
	float* pUVs
) const
{
	if (texInfoIdx < 0 || texInfoIdx > mNumTexInfos) return false;
	const TexInfo* pTexInfo = mpTexInfos + texInfoIdx;

	if (pTexInfo->texData < 0 || pTexInfo->texData >= mNumTexDatas) return false;
	const TexData* pTexData = mpTexDatas + pTexInfo->texData;

	const float* s = pTexInfo->textureVecs[0];
	const float* t = pTexInfo->textureVecs[1];

	pUVs[0] = s[0] * pos[0] + s[1] * pos[1] + s[2] * pos[2] + s[3];
	pUVs[1] = t[0] * pos[0] + t[1] * pos[1] + t[2] * pos[2] + t[3];

	pUVs[0] /= static_cast<float>(pTexData->width);
	pUVs[1] /= static_cast<float>(pTexData->height);

	return true;
}

bool BSPMap::GetSurfEdgeVerts(const int32_t index, float* pVertA, float* pVertB) const
{
	if (index < 0 || index >= mNumSurfEdges) return false;

	int32_t edgeIdx = mpSurfEdges[index];
	if (abs(edgeIdx) > mNumEdges) return false;

	uint16_t iA = mpEdges[abs(edgeIdx)].vertices[0];
	uint16_t iB = mpEdges[abs(edgeIdx)].vertices[1];

	if (iA >= mNumVertices || iB >= mNumVertices) return false;

	if (edgeIdx < 0) std::swap(iA, iB);

	memcpy(pVertA, mpVertices + iA, 3 * sizeof(float));
	if (pVertB != nullptr)
		memcpy(pVertB, mpVertices + iB, 3 * sizeof(float));

	return true;
}

bool BSPMap::Triangulate()
{
	// Calculate number of tris in the map
	mNumTris = 0U;
	for (const Face* pFace = mpFaces; pFace < mpFaces + mNumFaces; pFace++) {
		if (IsFaceNodraw(pFace) || pFace->numEdges < 3) continue;
		mNumTris += pFace->numEdges - 2;
	}
	if (mNumTris == 0U) return false;

	// malloc buffers
	mpPositions  = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * 3U * mNumTris));
	mpNormals    = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * mNumTris));
	mpTangents   = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * mNumTris));
	mpBinormals  = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * mNumTris));
	mpUVs        = reinterpret_cast<float*>(malloc(sizeof(float) * 2U * 3U * mNumTris));
	mpTexIndices = reinterpret_cast<uint32_t*>(malloc(sizeof(uint32_t) * mNumTris));

	if (
		mpPositions  == nullptr ||
		mpNormals    == nullptr ||
		mpTangents   == nullptr ||
		mpBinormals  == nullptr ||
		mpUVs        == nullptr ||
		mpTexIndices == nullptr
	) {
		FreeAll();
		return false;
	}

	// Read data into buffers
	size_t triIdx = 0U;
	for (const Face* pFace = mpFaces; pFace < mpFaces + mNumFaces; pFace++) {
		if (IsFaceNodraw(pFace) || pFace->numEdges < 3) continue;

		// Get ref to root vertex
		float* root = new float[3], * rootUV = new float[2];
		GetSurfEdgeVerts(pFace->firstEdge, root);
		CalcUVs(pFace->texInfo, root, rootUV);

		// Get normal
		int16_t planeIdx = pFace->planeNum;
		if (planeIdx < 0 || planeIdx > mNumPlanes) {
			FreeAll();
			return false;
		}

		Vector normal = mpPlanes[planeIdx].normal;
		if (pFace->side != 0) {
			normal.x = -normal.x;
			normal.y = -normal.y;
			normal.z = -normal.z;
		}

		// Get texture index
		uint32_t texIdx = pFace->texInfo;

		// For each edge (ignoring first and last)
		for (
			int32_t surfEdgeIdx = pFace->firstEdge + 1;
			surfEdgeIdx < pFace->firstEdge + pFace->numEdges - 1;
			surfEdgeIdx++
		) {
			// Offsets
			float* p0 = mpPositions + triIdx * 3U * 3U;
			float* p1 = mpPositions + triIdx * 3U * 3U + 3U;
			float* p2 = mpPositions + triIdx * 3U * 3U + 6U;

			float* n = mpNormals   + triIdx * 3U;
			float* t = mpTangents  + triIdx * 3U;
			float* b = mpBinormals + triIdx * 3U;

			float* uv0 = mpUVs + triIdx * 3U * 2U;
			float* uv1 = mpUVs + triIdx * 3U * 2U + 2U;
			float* uv2 = mpUVs + triIdx * 3U * 2U + 4U;

			// Add vertices to positions
			memcpy(p0, root, 3U * sizeof(float));
			if (!GetSurfEdgeVerts(surfEdgeIdx, p1, p2)) {
				FreeAll();
				return false;
			}

			// Calculate UVs
			memcpy(uv0, rootUV, 2U * sizeof(float));
			if (
				!CalcUVs(texIdx, p1, uv1) ||
				!CalcUVs(texIdx, p2, uv2)
			) {
				FreeAll();
				return false;
			}

			// Add normal and compute tangent/bitangent
			memcpy(n, &normal, 3U * sizeof(float));
			CalcTangentBinormal(
				p0, p1, p2,
				uv0, uv1, uv2,
				n, t, b
			);

			// Add texture index
			mpTexIndices[triIdx] = texIdx;

			// Increment
			triIdx++;
		}

		delete[] root;
	}

	return true;
}

BSPMap::BSPMap(
	const uint8_t* pFileData, const size_t dataSize
) : mDataSize(dataSize)
{
	if (pFileData == nullptr || dataSize == 0U) return;

	mpData = reinterpret_cast<uint8_t*>(malloc(dataSize));
	if (mpData == nullptr) return;
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
		return;
	}

	if (Triangulate()) mIsValid = true;
}

BSPMap::~BSPMap()
{
	FreeAll();
}

bool BSPMap::IsValid() const { return mIsValid; }

BSPTexture BSPMap::GetTexture(const uint32_t index) const
{
	if (index >= mNumTexInfos)
		throw std::runtime_error("Texture index out of bounds");

	const TexInfo* pTexInfo = mpTexInfos + index;

	if (pTexInfo->texData < 0 || pTexInfo->texData >= mNumTexDatas)
		throw std::runtime_error("TexData index out of bounds");
	const TexData* pTexData = mpTexDatas + pTexInfo->texData;

	if (
		pTexData->nameStringTableId < 0 ||
		pTexData->nameStringTableId >= mNumTexDataStringTableEntries
	) throw std::runtime_error("TexData string table index out of bounds");

	BSPTexture ret{};
	ret.flags = pTexInfo->flags;
	ret.reflectivity = pTexData->reflectivity;
	ret.path = mpTexDataStringData + mpTexDataStringTable[pTexData->nameStringTableId];
	ret.width = pTexData->width;
	ret.height = pTexData->height;

	return ret;
}

size_t BSPMap::GetNumTris() const { return mNumTris; }
const float* BSPMap::GetVertPositions() const { return mpPositions; }
const float* BSPMap::GetTriNormals() const { return mpNormals; }
const float* BSPMap::GetTriTangents() const { return mpTangents; }
const float* BSPMap::GetTriBinormals() const { return mpBinormals; }
const float* BSPMap::GetVertUVs() const { return mpUVs; }
const uint32_t* BSPMap::GetTriTextures() const { return mpTexIndices; }
