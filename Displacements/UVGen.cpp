#include "Displacements.h"

using namespace Displacements;
using namespace BSPStructs;

void Displacements::GenerateDispSurfUVs(const DispInfo* pDispInfo, float faceUVs[4][2], Displacement& disp)
{
	int postSpacing = (1 << pDispInfo->power) + 1;
	float ooInt = (1.0f / static_cast<float>(postSpacing - 1));

	float edgeInt[2][2];
	edgeInt[0][0] = (faceUVs[1][0] - faceUVs[0][0]) * ooInt;
	edgeInt[0][1] = (faceUVs[1][1] - faceUVs[0][1]) * ooInt;
	edgeInt[1][0] = (faceUVs[2][0] - faceUVs[3][0]) * ooInt;
	edgeInt[1][1] = (faceUVs[2][1] - faceUVs[3][1]) * ooInt;

	for (int i = 0; i < postSpacing; i++) {
		float endPts[2][2];
		endPts[0][0] = edgeInt[0][0] * i + faceUVs[0][0];
		endPts[0][1] = edgeInt[0][1] * i + faceUVs[0][1];
		endPts[1][0] = edgeInt[1][0] * i + faceUVs[3][0];
		endPts[1][1] = edgeInt[1][1] * i + faceUVs[3][1];

		float seg[2], segInt[2];
		seg[0] = endPts[1][0] - endPts[0][0];
		seg[1] = endPts[1][1] - endPts[0][1];
		segInt[0] = seg[0] * ooInt;
		segInt[1] = seg[1] * ooInt;

		for (int j = 0; j < postSpacing; j++) {
			seg[0] = segInt[0] * j;
			seg[1] = segInt[1] * j;

			int idx = (i * postSpacing + j) * 2;
			disp.uvs[idx] = endPts[0][0] + seg[0];
			disp.uvs[idx + 1] = endPts[0][1] + seg[1];
		}
	}
}
