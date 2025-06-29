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

bool BSPMap::IsFaceNodraw(const Face* pFace) const {
  return (
    pFace->texInfo < 0 || (mpTexInfos[pFace->texInfo].flags & (SURF::NODRAW | SURF::SKIP | SURF::TRIGGER)) != SURF::NONE
  );
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

  // Read data into buffers
  for (const Face* pFace = firstFace; pFace < firstFace + numFaces; pFace++) {
    if (IsFaceNodraw(pFace) || pFace->numEdges < 3) continue;

    // Get texture index
    int16_t texIdx = pFace->texInfo;
    if (texIdx < 0 || texIdx >= mNumTexInfos) continue;

    // Get displacement index
    int16_t dispIdx = pFace->dispInfo;

    if (dispIdx < 0) { // Triangulate face

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
