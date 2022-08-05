#include "Displacements.h"

#include <algorithm>

using namespace Displacements;
using namespace BSPStructs;

void Displacement::Init(const DispInfo* pDispInfo)
{
	pInfo = pDispInfo;
	size_t numVerts = (1 << pDispInfo->power) + 1;
	numVerts *= numVerts;

	verts = new Vector[numVerts];
	normals = new Vector[numVerts];
	tangents = new Vector[numVerts];
	binormals = new Vector[numVerts];

	uvs = new float[numVerts * 2U];
	alphas = new float[numVerts];
}

Displacement::~Displacement()
{
	if (verts != nullptr) delete[] verts;
	if (normals != nullptr) delete[] normals;
	if (tangents != nullptr) delete[] tangents;
	if (binormals != nullptr) delete[] binormals;
	if (uvs != nullptr) delete[] uvs;
	if (alphas != nullptr) delete[] alphas;
}

void Displacements::GenerateDispSurf(
	const DispInfo* pDispInfo, const DispVert* dispVerts,
	const Vector corners[4], Displacement& disp
)
{
	int postSpacing = (1 << pDispInfo->power) + 1;
	float ooInt = 1.0f / (float)(postSpacing - 1);

	Vector edgeInt[2];
	edgeInt[0] = corners[1] - corners[0];
	edgeInt[0] *= ooInt;

	edgeInt[1] = corners[2] - corners[3];
	edgeInt[1] *= ooInt;

	for (int i = 0; i < postSpacing; i++) {
		Vector endPts[2];
		endPts[0] = edgeInt[0] * i + corners[0];
		endPts[1] = edgeInt[1] * i + corners[3];

		Vector seg, segInt;
		seg = endPts[1] - endPts[0];
		segInt = seg * ooInt;

		for (int j = 0; j < postSpacing; j++) {
			int ndx = i * postSpacing + j;

			const DispVert* pDispInfoVert = dispVerts + ndx;
			disp.verts[ndx] = endPts[0] + segInt * j + pDispInfoVert->vec * pDispInfoVert->dist;
			disp.alphas[ndx] = std::clamp(pDispInfoVert->alpha / 255.f, 0.f, 1.f);
		}
	}
}
