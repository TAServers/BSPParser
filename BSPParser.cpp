#include "BSPParser.h"
#include "FileFormat/Structs.h"

#include <cstring>
#include <stdlib.h>
#include <cmath>
#include <stdexcept>
#include <limits>

using namespace BSPStructs;
using namespace BSPEnums;

inline float Dot(const float a[3], const float b[3])
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline void Cross(const float a[3], const float b[3], float out[3])
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}

inline void NegCross(const float a[3], const float b[3], float out[3])
{
	out[0] = a[2] * b[1] - a[1] * b[2];
	out[1] = a[0] * b[2] - a[2] * b[0];
	out[2] = a[1] * b[0] - a[0] * b[1];
}

inline void Normalise(float v[3])
{
	float len = sqrtf(Dot(v, v));
	v[0] /= len;
	v[1] /= len;
	v[2] /= len;
}

inline void Orthogonalise(float v[3], const float basis[3])
{
	float dot = Dot(v, basis);
	v[0] -= basis[0] * dot;
	v[1] -= basis[1] * dot;
	v[2] -= basis[2] * dot;

	Normalise(v);
}

void CalcNormal(
	const float p0[3], const float p1[3], const float p2[3],
	float n[3]
)
{
	float edge0[3] = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
	float edge1[3] = { p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2] };

	Cross(edge0, edge1, n);
	Normalise(n);
}

void CalcTangentBinormal(
	const float p0[3], const float p1[3], const float p2[3],
	const float uv0[2], const float uv1[2], const float uv2[2],
	const float n[3], float t[3], float b[3]
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

void CalcTangentBinormal(
	const float p0[3], const float p1[3], const float p2[3],
	const float uv0[2], const float uv1[2], const float uv2[2],
	const float n0[3], const float n1[3], const float n2[3],
	float t0[3], float t1[3], float t2[3],
	float b0[3], float b1[3], float b2[3]
)
{
	float edge0[3] = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
	float edge1[3] = { p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2] };

	float dUV0[2] = { uv1[0] - uv0[0], uv1[1] - uv0[1] };
	float dUV1[2] = { uv2[0] - uv0[0], uv2[1] - uv0[1] };

	float f = 1.f / (dUV0[0] * dUV1[1] - dUV1[0] * dUV0[1]);

	float t[3] = {
		f * (dUV1[1] * edge0[0] - dUV0[1] * edge1[0]),
		f * (dUV1[1] * edge0[1] - dUV0[1] * edge1[1]),
		f * (dUV1[1] * edge0[2] - dUV0[1] * edge1[2])
	};

	memcpy(t0, t, 3 * sizeof(float));
	memcpy(t1, t, 3 * sizeof(float));
	memcpy(t2, t, 3 * sizeof(float));

	Orthogonalise(t0, n0);
	Orthogonalise(t1, n1);
	Orthogonalise(t2, n2);

	Cross(t0, n0, b0);
	Cross(t1, n1, b1);
	Cross(t2, n2, b2);
}

void CalcVertWNormals(
	const float p0[3], const float p1[3], const float p2[3],
	float n0[3], float n1[3], float n2[3]
) {
	float normal[3];
	CalcNormal(p0, p1, p2, normal);

	float p1p0[3] = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
	float p2p0[3] = { p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2] };
	float p2p1[3] = { p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2] };
	float p0p1[3] = { p0[0] - p1[0], p0[1] - p1[1], p0[2] - p1[2] };
	float p0p2[3] = { p0[0] - p2[0], p0[1] - p2[1], p0[2] - p2[2] };
	float p1p2[3] = { p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2] };

	// TODO: No need to compute the length twice for each inverted edge
	Normalise(p1p0);
	Normalise(p2p0);
	Normalise(p2p1);
	Normalise(p0p1);
	Normalise(p0p2);
	Normalise(p1p2);

	float a0 = acosf(Dot(p1p0, p2p0));
	float a1 = acosf(Dot(p2p1, p0p1));
	float a2 = acosf(Dot(p0p2, p1p2));

	for (int i = 0; i < 3; i++) {
		n0[i] += normal[i] * a0;
		n1[i] += normal[i] * a1;
		n2[i] += normal[i] * a2;
	}
}

bool BSPMap::IsFaceNodraw(const Face* pFace) const
{
	return (
		pFace->texInfo < 0 ||
		(
			mpTexInfos[pFace->texInfo].flags &
			static_cast<int32_t>(
				SURF::NODRAW |
				SURF::SKIP |
				SURF::HITBOX |
				SURF::TRIGGER
			)
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
	const int16_t texInfoIdx, const float* pos,
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

bool BSPMap::GenerateDispVert(
	const int32_t dispVertStart,
	int32_t x, int32_t y, int32_t size,
	const float* corners, int32_t firstCorner,
	float* pVert
) const
{
	int32_t offset = dispVertStart + x + y * (size + 1);
	if (offset < 0 || offset >= mNumDispVerts) {
		return false;
	}
	const DispVert* pDispVert = mpDispVerts + offset;

	float tx = static_cast<float>(x) / static_cast<float>(size);
	float ty = static_cast<float>(y) / static_cast<float>(size);
	float sx = 1.f - tx, sy = 1.f - ty;

	const float* c0 = corners + ((0 + firstCorner) & 3) * 3;
	const float* c1 = corners + ((1 + firstCorner) & 3) * 3;
	const float* c2 = corners + ((2 + firstCorner) & 3) * 3;
	const float* c3 = corners + ((3 + firstCorner) & 3) * 3;

	for (int i = 0; i < 3; i++) {
		pVert[i] = (c1[i] * sx + c2[i] * tx) * ty + (c0[i] * sx + c3[i] * tx) * sy;
		pVert[i] += reinterpret_cast<const float*>(&pDispVert->vec)[i] * pDispVert->dist;
	}

	return true;
}

bool BSPMap::Triangulate()
{
	// Get worldspawn faces
	const Face* firstFace = mpFaces + mpModels[0].firstFace;
	int32_t numFaces = mpModels[0].numFaces;

	// Calculate number of tris in the map
	mNumTris = 0U;
	for (const Face* pFace = firstFace; pFace < firstFace + numFaces; pFace++) {
		if (IsFaceNodraw(pFace) || pFace->numEdges < 3) continue;

		int16_t dispIdx = pFace->dispInfo;
		if (dispIdx < 0) { // Not a displacement
			mNumTris += pFace->numEdges - 2;
		} else {
			if (dispIdx >= mNumDispInfos) return false;

			int32_t size = 1 << mpDispInfos[dispIdx].power;
			mNumTris += size * size * 2;
		}
	}
	if (mNumTris == 0U) return false;

	// malloc buffers
	mpPositions  = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * 3U * mNumTris));
	mpNormals    = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * 3U * mNumTris));
	mpTangents   = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * 3U * mNumTris));
	mpBinormals  = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * 3U * mNumTris));
	mpUVs        = reinterpret_cast<float*>(malloc(sizeof(float) * 2U * 3U * mNumTris));
	mpTexIndices = reinterpret_cast<int16_t*>(malloc(sizeof(int16_t) * mNumTris));

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

	// Offsets
	size_t triIdx = 0U;

	float* p0 = mpPositions;
	float* p1 = mpPositions + 3U;
	float* p2 = mpPositions + 6U;

	float* n0 = mpNormals;
	float* n1 = mpNormals + 3U;
	float* n2 = mpNormals + 6U;

	float* t0 = mpTangents;
	float* t1 = mpTangents + 3U;
	float* t2 = mpTangents + 6U;

	float* b0 = mpBinormals;
	float* b1 = mpBinormals + 3U;
	float* b2 = mpBinormals + 6U;

	float* uv0 = mpUVs;
	float* uv1 = mpUVs + 2U;
	float* uv2 = mpUVs + 4U;

	// Read data into buffers
	for (const Face* pFace = firstFace; pFace < firstFace + numFaces; pFace++) {
		if (IsFaceNodraw(pFace) || pFace->numEdges < 3) continue;

		// Get texture index
		int16_t texIdx = pFace->texInfo;
		if (texIdx < 0) continue;

		// Get displacement index
		int16_t dispIdx = pFace->dispInfo;

		if (dispIdx < 0) { // Triangulate face
			// Get root vertex
			float root[3], rootUV[2];
			GetSurfEdgeVerts(pFace->firstEdge, root);
			if (!CalcUVs(pFace->texInfo, root, rootUV)) {
				FreeAll();
				return false;
			}

			// For each edge (ignoring first and last)
			for (
				int32_t surfEdgeIdx = pFace->firstEdge + 1;
				surfEdgeIdx < pFace->firstEdge + pFace->numEdges - 1;
				surfEdgeIdx++
			) {
				// Add vertices to positions
				memcpy(p0, root, 3U * sizeof(float));
				if (
					(mClockwise && !GetSurfEdgeVerts(surfEdgeIdx, p1, p2)) ||
					(!mClockwise && !GetSurfEdgeVerts(surfEdgeIdx, p2, p1))
				) {
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

				// Compute normal/tangent/bitangent
				CalcNormal(p0, p1, p2, n0);
				CalcTangentBinormal(
					p0, p1, p2,
					uv0, uv1, uv2,
					n0, t0, b0
				);
				memcpy(n1, n0, 3U * sizeof(float));
				memcpy(n2, n0, 3U * sizeof(float));
				memcpy(t1, t0, 3U * sizeof(float));
				memcpy(t2, t0, 3U * sizeof(float));
				memcpy(b1, b0, 3U * sizeof(float));
				memcpy(b2, b0, 3U * sizeof(float));

				// Add texture index
				mpTexIndices[triIdx] = texIdx;

				// Increment
				triIdx++;

				p0 += 3U * 3U;
				p1 += 3U * 3U;
				p2 += 3U * 3U;

				n0 += 3U * 3U;
				n1 += 3U * 3U;
				n2 += 3U * 3U;

				t0 += 3U * 3U;
				t1 += 3U * 3U;
				t2 += 3U * 3U;

				b0 += 3U * 3U;
				b1 += 3U * 3U;
				b2 += 3U * 3U;

				uv0 += 3U * 2U;
				uv1 += 3U * 2U;
				uv2 += 3U * 2U;
			}
		} else { // Triangulate displacement (https://github.com/Galaco/kero/blob/master/scene/loaders/bsp.go#L255)
			const DispInfo* pDispInfo = mpDispInfos + dispIdx;
			int32_t size = 1 << pDispInfo->power;

			float corners[4 * 3];
			int32_t firstCorner = 0;
			float firstCornerDist2 = std::numeric_limits<float>::max();

			for (
				int32_t surfEdgeIdx = pFace->firstEdge;
				surfEdgeIdx < pFace->firstEdge + pFace->numEdges;
				surfEdgeIdx++
			) {
				float vert[3];
				if (!GetSurfEdgeVerts(surfEdgeIdx, vert)) {
					FreeAll();
					return false;
				}
				memcpy(corners + (surfEdgeIdx - pFace->firstEdge) * 3U, vert, 3U * sizeof(float));

				float distVec[3] = {
					pDispInfo->startPosition.x - vert[0],
					pDispInfo->startPosition.y - vert[1],
					pDispInfo->startPosition.z - vert[2]
				};
				float dist2 = distVec[0] * distVec[0] + distVec[1] * distVec[1] + distVec[2] * distVec[2];
				if (dist2 < firstCornerDist2) {
					firstCorner = surfEdgeIdx - pFace->firstEdge;
					firstCornerDist2 = dist2;
				}
			}

			float* dispVerts       = new float[(size + 1) * (size + 1) * 3];
			float* dispVertNormals = new float[(size + 1) * (size + 1) * 3];

			for (int32_t x = 0; x < size; x++) {
				for (int32_t y = 0; y < size; y++) {
					int32_t a = y * (size + 1) * 3 + x * 3;
					int32_t b = (y + 1) * (size + 1) * 3 + x * 3;
					int32_t c = (y + 1) * (size + 1) * 3 + (x + 1) * 3;
					int32_t d = y * (size + 1) * 3 + (x + 1) * 3;
					
					// Calculate displacement vertices
					// TOOD: This could be optimised to only generate each vert once with some conditions
					if (
						!GenerateDispVert(
							pDispInfo->dispVertStart,
							x, y, size,
							corners, firstCorner,
							dispVerts + a
						) ||
						!GenerateDispVert(
							pDispInfo->dispVertStart,
							x, y + 1, size,
							corners, firstCorner,
							dispVerts + b
						) ||
						!GenerateDispVert(
							pDispInfo->dispVertStart,
							x + 1, y + 1, size,
							corners, firstCorner,
							dispVerts + c
						) ||
						!GenerateDispVert(
							pDispInfo->dispVertStart,
							x + 1, y, size,
							corners, firstCorner,
							dispVerts + d
						)
					) {
						FreeAll();
						return false;
					}

					// Add tris to vertex normals
					CalcVertWNormals(
						dispVerts + a,
						dispVerts + b,
						dispVerts + c,
						dispVertNormals + a,
						dispVertNormals + b,
						dispVertNormals + c
					);
					CalcVertWNormals(
						dispVerts + a,
						dispVerts + c,
						dispVerts + d,
						dispVertNormals + a,
						dispVertNormals + c,
						dispVertNormals + d
					);
				}
			}

			// Normalise vert normals
			for (
				float* normal = dispVertNormals;
				normal < dispVertNormals + (size + 1) * (size + 1) * 3;
				normal += 3
			) {
				Normalise(normal);
				if (!mClockwise) {
					normal[0] = -normal[0];
					normal[1] = -normal[1];
					normal[2] = -normal[2];
				}
			}

			// Write tris
			for (int32_t x = 0; x < size; x++) {
				for (int32_t y = 0; y < size; y++) {
					int32_t a = y * (size + 1) * 3 + x * 3;
					int32_t b = (y + 1) * (size + 1) * 3 + x * 3;
					int32_t c = (y + 1) * (size + 1) * 3 + (x + 1) * 3;
					int32_t d = y * (size + 1) * 3 + (x + 1) * 3;

					for (int tri = 0; tri < 2; tri++) {
						memcpy(p0, dispVerts + a, 3U * sizeof(float));
						memcpy(mClockwise ? p1 : p2, dispVerts + (tri == 0 ? b : c), 3U * sizeof(float));
						memcpy(mClockwise ? p2 : p1, dispVerts + (tri == 0 ? c : d), 3U * sizeof(float));

						if (
							!CalcUVs(texIdx, p0, uv0) ||
							!CalcUVs(texIdx, p1, uv1) ||
							!CalcUVs(texIdx, p2, uv2)
						) {
							FreeAll();
							return false;
						}

						// Add normals/tangents/binormals
						memcpy(n0, dispVertNormals + a, 3 * sizeof(float));
						memcpy(mClockwise ? n1 : n2, dispVertNormals + (tri == 0 ? b : c), 3 * sizeof(float));
						memcpy(mClockwise ? n2 : n1, dispVertNormals + (tri == 0 ? c : d), 3 * sizeof(float));

						CalcTangentBinormal(
							p0, p1, p2,
							uv0, uv1, uv2,
							n0, n1, n2,
							t0, t1, t2,
							b0, b1, b2
						);

						// Increment
						triIdx++;

						p0 += 3U * 3U;
						p1 += 3U * 3U;
						p2 += 3U * 3U;

						n0 += 3U * 3U;
						n1 += 3U * 3U;
						n2 += 3U * 3U;

						t0 += 3U * 3U;
						t1 += 3U * 3U;
						t2 += 3U * 3U;

						b0 += 3U * 3U;
						b1 += 3U * 3U;
						b2 += 3U * 3U;

						uv0 += 3U * 2U;
						uv1 += 3U * 2U;
						uv2 += 3U * 2U;
					}
				}
			}

			delete[] dispVertNormals;
		}
	}

	return true;
}

BSPMap::BSPMap(
	const uint8_t* pFileData, const size_t dataSize, const bool clockwise
) : mDataSize(dataSize), mClockwise(clockwise)
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
		) ||
		!ParseLump(&mpModels, &mNumModels) ||
		!ParseLump(&mpDispInfos, &mNumDispInfos) ||
		!ParseLump(&mpDispVerts, &mNumDispVerts)
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

BSPTexture BSPMap::GetTexture(const int16_t index) const
{
	if (index < 0 || index >= mNumTexInfos)
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
const float* BSPMap::GetVertices() const { return mpPositions; }
const float* BSPMap::GetNormals() const { return mpNormals; }
const float* BSPMap::GetTangents() const { return mpTangents; }
const float* BSPMap::GetBinormals() const { return mpBinormals; }
const float* BSPMap::GetUVs() const { return mpUVs; }
const int16_t* BSPMap::GetTriTextures() const { return mpTexIndices; }
