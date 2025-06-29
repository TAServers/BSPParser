#include "BSPParser.h"
#include "Displacements/Displacements.h"
#include "Errors/ParseError.hpp"
#include "Errors/TriangulationError.hpp"
#include "FileFormat/Structs.h"
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>

using namespace BSPStructs;
using namespace BSPEnums;
using namespace BSPErrors;

void CalcNormal(const Vector* p0, const Vector* p1, const Vector* p2, Vector* n) {
  Vector edge0 = {p1->x - p0->x, p1->y - p0->y, p1->z - p0->z};
  Vector edge1 = {p2->x - p0->x, p2->y - p0->y, p2->z - p0->z};

  *n = edge1.Cross(edge0);
  n->Normalise();
}

void CalcTangentBinormal(const TexInfo* pTexInfo, const Plane* pPlane, const Vector* n, Vector* t, Vector* b) {
  Vector sAxis{pTexInfo->textureVecs[0][0], pTexInfo->textureVecs[0][1], pTexInfo->textureVecs[0][2]};
  Vector tAxis{pTexInfo->textureVecs[1][0], pTexInfo->textureVecs[1][1], pTexInfo->textureVecs[1][2]};

  *b = tAxis;
  b->Normalise();
  *t = n->Cross(*b);
  t->Normalise();
  *b = t->Cross(*n);
  b->Normalise();

  if (pPlane->normal.Dot(sAxis.Cross(tAxis)) > 0.0f) {
    *t *= -1;
  }
}

bool BSPMap::IsFaceNodraw(const Face* pFace) const {
  return (
    pFace->texInfo < 0 || (mpTexInfos[pFace->texInfo].flags & (SURF::NODRAW | SURF::SKIP | SURF::TRIGGER)) != SURF::NONE
  );
}

bool BSPMap::CalcUVs(const int16_t texInfoIdx, const Vector* const pos, float* const pUVs) const {
  if (texInfoIdx < 0 || texInfoIdx > mNumTexInfos) return false;
  const TexInfo* pTexInfo = mpTexInfos + texInfoIdx;

  if (pTexInfo->texData < 0 || pTexInfo->texData >= mNumTexDatas) return false;
  const TexData* pTexData = mpTexDatas + pTexInfo->texData;

  const float* s = pTexInfo->textureVecs[0];
  const float* t = pTexInfo->textureVecs[1];

  pUVs[0] = s[0] * pos->x + s[1] * pos->y + s[2] * pos->z + s[3];
  pUVs[1] = t[0] * pos->x + t[1] * pos->y + t[2] * pos->z + t[3];

  pUVs[0] /= static_cast<float>(pTexData->width);
  pUVs[1] /= static_cast<float>(pTexData->height);

  return true;
}

bool BSPMap::GetSurfEdgeVerts(const int32_t index, Vector* pVertA, Vector* pVertB) const {
  if (index < 0 || index >= mNumSurfEdges) return false;

  int32_t edgeIdx = mpSurfEdges[index];
  if (abs(edgeIdx) > mNumEdges) return false;

  uint16_t iA = mpEdges[abs(edgeIdx)].vertices[0];
  uint16_t iB = mpEdges[abs(edgeIdx)].vertices[1];

  if (iA >= mNumVertices || iB >= mNumVertices) return false;

  if (edgeIdx < 0) std::swap(iA, iB);

  memcpy(pVertA, mpVertices + iA, sizeof(Vector));
  if (pVertB != nullptr) memcpy(pVertB, mpVertices + iB, sizeof(Vector));

  return true;
}

void BSPMap::Triangulate() {
  using Displacement = Displacements::Displacement;

  // Get worldspawn faces
  const Face* firstFace = mpFaces + mpModels[0].firstFace;
  int32_t numFaces = mpModels[0].numFaces;

  // Calculate number of tris in the map
  mNumTris = 0U;
  for (const Face* pFace = firstFace; pFace < firstFace + numFaces; pFace++) {
    if (IsFaceNodraw(pFace) || pFace->numEdges < 3) continue;

    int16_t texIdx = pFace->texInfo;
    if (texIdx < 0 || texIdx >= mNumTexInfos) continue;

    int16_t dispIdx = pFace->dispInfo;
    if (dispIdx < 0) { // Not a displacement
      mNumTris += pFace->numEdges - 2;
    } else {
      if (dispIdx >= mNumDispInfos) {
        throw TriangulationError("Displacement index is greater than the number of displacements");
      }

      int32_t size = 1 << mpDispInfos[dispIdx].power;
      mNumTris += size * size * 2;
    }
  }
  if (mNumTris == 0U) {
    throw TriangulationError("Map has no triangles");
  }

  // malloc buffers
  mpPositions = reinterpret_cast<Vector*>(malloc(sizeof(Vector) * 3U * mNumTris));
  mpNormals = reinterpret_cast<Vector*>(malloc(sizeof(Vector) * 3U * mNumTris));
  mpTangents = reinterpret_cast<Vector*>(malloc(sizeof(Vector) * 3U * mNumTris));
  mpBinormals = reinterpret_cast<Vector*>(malloc(sizeof(Vector) * 3U * mNumTris));
  mpUVs = reinterpret_cast<float*>(malloc(sizeof(float) * 2U * 3U * mNumTris));
  mpAlphas = reinterpret_cast<float*>(malloc(sizeof(float) * 3U * mNumTris));
  mpTexIndices = reinterpret_cast<int16_t*>(malloc(sizeof(int16_t) * mNumTris));

  if (mpPositions == nullptr || mpNormals == nullptr || mpTangents == nullptr || mpBinormals == nullptr ||
      mpUVs == nullptr || mpAlphas == nullptr || mpTexIndices == nullptr) {
    FreeAll();
    throw TriangulationError("Failed to allocate memory for triangle data");
  }

  // Generate vertices from displacements and smooth normals
  std::vector<Displacement> displacements(mNumDispInfos);
  for (int dispIdx = 0; dispIdx < mNumDispInfos; dispIdx++) {
    const DispInfo* pDispInfo = mpDispInfos + dispIdx;
    const Face* pFace = mpFaces + pDispInfo->mapFace;

    Vector corners[4];
    int32_t firstCorner = 0;
    float firstCornerDist2 = std::numeric_limits<float>::max();

    for (int32_t surfEdgeIdx = pFace->firstEdge; surfEdgeIdx < pFace->firstEdge + pFace->numEdges; surfEdgeIdx++) {
      Vector vert;
      if (!GetSurfEdgeVerts(surfEdgeIdx, &vert)) {
        FreeAll();
        throw TriangulationError("Failed to get surface edge vertices");
      }
      corners[surfEdgeIdx - pFace->firstEdge] = vert;

      Vector distVec = pDispInfo->startPosition - vert;
      float dist2 = distVec.Dot(distVec);
      if (dist2 < firstCornerDist2) {
        firstCorner = surfEdgeIdx - pFace->firstEdge;
        firstCornerDist2 = dist2;
      }
    }

    // Reorder corners
    {
      Vector tmpPoints[4];
      for (int i = 0; i < 4; i++) {
        tmpPoints[i] = corners[i];
      }

      for (int i = 0; i < 4; i++) {
        corners[i] = tmpPoints[(i + firstCorner) % 4];
      }
    }

    Displacement& disp = displacements[dispIdx];
    disp.Init(pDispInfo);

    Displacements::GenerateDispSurf(pDispInfo, mpDispVerts + pDispInfo->dispVertStart, corners, disp);
    Displacements::GenerateDispSurfNormals(pDispInfo, disp);
    Displacements::GenerateDispSurfTangentSpaces(
      pDispInfo, mpPlanes + pFace->planeNum, mpTexInfos + pFace->texInfo, disp
    );

    float faceUVs[4][2];
    for (int i = 0; i < 4; i++) {
      CalcUVs(pFace->texInfo, corners + i, faceUVs[i]);
    }

    Displacements::GenerateDispSurfUVs(pDispInfo, faceUVs, disp);
  }

  try {
    Displacements::SmoothNeighbouringDispSurfNormals(displacements);
  } catch (const std::out_of_range& e) {
    FreeAll();
    throw TriangulationError(e.what());
  }

  // Offsets
  size_t triIdx = 0U;

  Vector* p0 = mpPositions;
  Vector* p1 = mpPositions + 1U;
  Vector* p2 = mpPositions + 2U;

  Vector* n0 = mpNormals;
  Vector* n1 = mpNormals + 1U;
  Vector* n2 = mpNormals + 2U;

  Vector* t0 = mpTangents;
  Vector* t1 = mpTangents + 1U;
  Vector* t2 = mpTangents + 2U;

  Vector* b0 = mpBinormals;
  Vector* b1 = mpBinormals + 1U;
  Vector* b2 = mpBinormals + 2U;

  float* uv0 = mpUVs;
  float* uv1 = mpUVs + 2U;
  float* uv2 = mpUVs + 4U;

  float* a0 = mpAlphas;
  float* a1 = mpAlphas + 1U;
  float* a2 = mpAlphas + 2U;

  // Read data into buffers
  for (const Face* pFace = firstFace; pFace < firstFace + numFaces; pFace++) {
    if (IsFaceNodraw(pFace) || pFace->numEdges < 3) continue;

    // Get texture index
    int16_t texIdx = pFace->texInfo;
    if (texIdx < 0 || texIdx >= mNumTexInfos) continue;

    // Get displacement index
    int16_t dispIdx = pFace->dispInfo;

    if (dispIdx < 0) { // Triangulate face
      // Get root vertex
      Vector root;
      float rootUV[2];
      GetSurfEdgeVerts(pFace->firstEdge, &root);
      if (!CalcUVs(pFace->texInfo, &root, rootUV)) {
        FreeAll();
        throw TriangulationError("Failed to calculate root face UVs");
      }

      // For each edge (ignoring first and last)
      for (int32_t surfEdgeIdx = pFace->firstEdge + 1; surfEdgeIdx < pFace->firstEdge + pFace->numEdges - 1;
           surfEdgeIdx++) {
        // Add vertices to positions
        *p0 = root;
        if ((mClockwise && !GetSurfEdgeVerts(surfEdgeIdx, p1, p2)) ||
            (!mClockwise && !GetSurfEdgeVerts(surfEdgeIdx, p2, p1))) {
          FreeAll();
          throw TriangulationError("Failed to get surface edge vertices");
        }

        // Calculate UVs
        memcpy(uv0, rootUV, 2U * sizeof(float));
        if (!CalcUVs(texIdx, p1, uv1) || !CalcUVs(texIdx, p2, uv2)) {
          FreeAll();
          throw TriangulationError("Failed to calculate face vertex UVs");
        }

        // Compute normal/tangent/bitangent
        CalcNormal(p0, p1, p2, n0);
        CalcTangentBinormal(mpTexInfos + texIdx, mpPlanes + pFace->planeNum, n0, t0, b0);
        *n1 = *n0;
        *n2 = *n0;
        *t1 = *t0;
        *t2 = *t0;
        *b1 = *b0;
        *b2 = *b0;

        *a0 = 1.f;
        *a1 = 1.f;
        *a2 = 1.f;

        // Add texture index
        mpTexIndices[triIdx] = texIdx;

        // Increment
        triIdx++;

        p0 += 3U;
        p1 += 3U;
        p2 += 3U;

        n0 += 3U;
        n1 += 3U;
        n2 += 3U;

        t0 += 3U;
        t1 += 3U;
        t2 += 3U;

        b0 += 3U;
        b1 += 3U;
        b2 += 3U;

        uv0 += 3U * 2U;
        uv1 += 3U * 2U;
        uv2 += 3U * 2U;

        a0 += 3U;
        a1 += 3U;
        a2 += 3U;
      }
    } else { // Triangulate displacement
      const Displacement& disp = displacements[dispIdx];
      int32_t size = 1 << disp.pInfo->power;

      // Write tris
      for (int32_t x = 0; x < size; x++) {
        for (int32_t y = 0; y < size; y++) {
          int32_t a = y * (size + 1) + x;
          int32_t b = (y + 1) * (size + 1) + x;
          int32_t c = (y + 1) * (size + 1) + (x + 1);
          int32_t d = y * (size + 1) + (x + 1);

          for (int tri = 0; tri < 2; tri++) {
            int32_t i0 = a, i1 = tri == 0 ? b : c, i2 = tri == 0 ? c : d;
            if (!mClockwise) std::swap(i1, i2);

            *p0 = disp.verts[i0];
            *p1 = disp.verts[i1];
            *p2 = disp.verts[i2];

            *n0 = disp.normals[i0];
            *n1 = disp.normals[i1];
            *n2 = disp.normals[i2];

            *t0 = disp.tangents[i0];
            *t1 = disp.tangents[i1];
            *t2 = disp.tangents[i2];

            *b0 = disp.binormals[i0];
            *b1 = disp.binormals[i1];
            *b2 = disp.binormals[i2];

            memcpy(uv0, disp.uvs + i0 * 2, sizeof(float) * 2);
            memcpy(uv1, disp.uvs + i1 * 2, sizeof(float) * 2);
            memcpy(uv2, disp.uvs + i2 * 2, sizeof(float) * 2);

            *a0 = disp.alphas[i0];
            *a1 = disp.alphas[i1];
            *a2 = disp.alphas[i2];

            // Add texture index
            mpTexIndices[triIdx] = texIdx;

            // Increment
            triIdx++;

            p0 += 3U;
            p1 += 3U;
            p2 += 3U;

            n0 += 3U;
            n1 += 3U;
            n2 += 3U;

            t0 += 3U;
            t1 += 3U;
            t2 += 3U;

            b0 += 3U;
            b1 += 3U;
            b2 += 3U;

            uv0 += 3U * 2U;
            uv1 += 3U * 2U;
            uv2 += 3U * 2U;

            a0 += 3U;
            a1 += 3U;
            a2 += 3U;
          }
        }
      }
    }
  }
}

BSPTexture BSPMap::GetTexture(const int16_t index) const {
  if (index < 0 || index >= mNumTexInfos) throw std::out_of_range("Texture index out of bounds");

  const TexInfo* pTexInfo = mpTexInfos + index;

  if (pTexInfo->texData < 0 || pTexInfo->texData >= mNumTexDatas)
    throw std::out_of_range("TexData index out of bounds");
  const TexData* pTexData = mpTexDatas + pTexInfo->texData;

  if (pTexData->nameStringTableId < 0 || pTexData->nameStringTableId >= mNumTexDataStringTableEntries)
    throw std::out_of_range("TexData string table index out of bounds");

  BSPTexture ret{};
  ret.flags = pTexInfo->flags;
  ret.reflectivity = pTexData->reflectivity;
  ret.path = mpTexDataStringData + mpTexDataStringTable[pTexData->nameStringTableId];
  ret.width = pTexData->width;
  ret.height = pTexData->height;

  return ret;
}

BSPStaticProp BSPMap::GetStaticProp(const int32_t index) const {
  switch (mStaticPropsVersion) {
    case 4:
      return GetStaticPropInternal(index, mpStaticPropsV4);
    case 5:
      return GetStaticPropInternal(index, mpStaticPropsV5);
    case 6:
      return GetStaticPropInternal(index, mpStaticPropsV6);
    case 7:
      return GetStaticPropInternal(index, mpStaticPropsV7Multiplayer2013);
    default:
      throw std::runtime_error("Unsupported static prop version");
  }
}
