#include "BSPParser.h"
#include "Displacements/Displacements.h"
#include "Errors/ParseError.hpp"
#include "Errors/TriangulationError.hpp"
#include "FileFormat/Structs.h"
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>

void BSPMap::Triangulate() {
  using Displacement = Displacements::Displacement;

  // Generate vertices from displacements and smooth normals
  std::vector<Displacement> displacements(mNumDispInfos);
  for (int dispIdx = 0; dispIdx < mNumDispInfos; dispIdx++) {
    const DispInfo* pDispInfo = mpDispInfos + dispIdx;
    const Face* pFace = mpFaces + pDispInfo->mapFace;

    Vector corners[4];

    Displacement& disp = displacements[dispIdx];
    disp.Init(pDispInfo);

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
}
