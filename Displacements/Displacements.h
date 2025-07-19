#pragma once

/*
	Implements builddisp.cpp and displacement normal smoothing from the Source SDK
*/

#include "FileFormat/Structs.h"
#include <vector>

namespace Displacements {
  constexpr char CORNER_LOWER_LEFT = 0;
  constexpr char CORNER_UPPER_LEFT = 1;
  constexpr char CORNER_UPPER_RIGHT = 2;
  constexpr char CORNER_LOWER_RIGHT = 3;

  constexpr char NEIGHBOREDGE_LEFT = 0;
  constexpr char NEIGHBOREDGE_TOP = 1;
  constexpr char NEIGHBOREDGE_RIGHT = 2;
  constexpr char NEIGHBOREDGE_BOTTOM = 3;

  static int g_EdgeDims[4] = {
    0, // NEIGHBOREDGE_LEFT   = X
    1, // NEIGHBOREDGE_TOP    = Y
    0, // NEIGHBOREDGE_RIGHT  = X
    1 // NEIGHBOREDGE_BOTTOM = Y
  };

  struct VertIndex {
    short x, y;
    inline short& operator[](short i) {
      return reinterpret_cast<short*>(this)[i];
    }
    inline const short& operator[](short i) const {
      return reinterpret_cast<const short*>(this)[i];
    }
  };

  struct Displacement {
    const BSPStructs::DispInfo* pInfo = nullptr;

    BSPStructs::Vector* verts = nullptr;
    BSPStructs::Vector* normals = nullptr;
    BSPStructs::Vector* tangents = nullptr;
    BSPStructs::Vector* binormals = nullptr;

    float* uvs = nullptr;
    float* alphas = nullptr;

    void Init(const BSPStructs::DispInfo* pDispInfo);
    ~Displacement();
  };

  void GenerateDispSurfNormals(const BSPStructs::DispInfo* pDispInfo, Displacement& disp);

  void GenerateDispSurfTangentSpaces(
    const BSPStructs::DispInfo* pDispInfo,
    const BSPStructs::Plane* pPlane,
    const BSPStructs::TexInfo* pTexInfo,
    Displacement& disp
  );

  void GenerateDispSurfUVs(const BSPStructs::DispInfo* pDispInfo, float faceUVs[4][2], Displacement& disp);

  void SmoothNeighbouringDispSurfNormals(std::vector<Displacement>& displacements);
}
