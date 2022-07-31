#pragma once

/*
	Implements builddisp.cpp and displacement normal smoothing from the Source SDK
*/

#include "FileFormat/Structs.h"

namespace Displacements
{
	void GenerateDispSurf(
		const BSPStructs::DispInfo* pDisp, const BSPStructs::DispVert* dispVerts,
		const BSPStructs::Vector corners[4],
		BSPStructs::Vector* pVerts
	);

	void GenerateDispSurfNormals(
		const BSPStructs::DispInfo* pDisp, const BSPStructs::Vector* pVerts,
		BSPStructs::Vector* pNormals
	);

	void GenerateDispSurfTangentSpaces(
		const BSPStructs::DispInfo* pDisp, const BSPStructs::TexInfo* pTexInfo, const BSPStructs::Plane* pPlane,
		const BSPStructs::Vector* pVerts, const BSPStructs::Vector* pNormals,
		BSPStructs::Vector* pTangents, BSPStructs::Vector* pBinormals
	);

	void SmoothNeighboringDispSurfNormals(const BSPStructs::DispInfo** pDisplacements, size_t numDisplacements);
}
