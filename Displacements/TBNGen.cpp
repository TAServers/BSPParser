#include "Displacements.h"

using namespace BSPStructs;

bool DoesEdgeExist(int indexRow, int indexCol, int direction, int postSpacing)
{
	switch (direction) {
	case 0:
		// left edge
		if ((indexRow - 1) < 0)
			return false;
		return true;
	case 1:
		// top edge
		if ((indexCol + 1) > (postSpacing - 1))
			return false;
		return true;
	case 2:
		// right edge
		if ((indexRow + 1) > (postSpacing - 1))
			return false;
		return true;
	case 3:
		// bottom edge
		if ((indexCol - 1) < 0)
			return false;
		return true;
	default:
		return false;
	}
}

Vector CalcNormalFromEdges(
	const DispInfo* pDispInfo, const std::vector<Vector>& verts,
	int indexRow, int indexCol, bool isEdge[4], int postSpacing
)
{
	Vector accumNormal{};
	int normalCount = 0;

	Vector tmpVec[2];
	Vector tmpNormal;

	if (isEdge[1] && isEdge[2]) {
		tmpVec[0] = verts.at((indexCol + 1) * postSpacing + indexRow) - verts.at(indexCol * postSpacing + indexRow);
		tmpVec[1] = verts.at(indexCol * postSpacing + (indexRow + 1)) - verts.at(indexCol * postSpacing + indexRow);
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;

		tmpVec[0] = verts.at((indexCol + 1) * postSpacing + indexRow) - verts.at(indexCol * postSpacing + (indexRow + 1));
		tmpVec[1] = verts.at((indexCol + 1) * postSpacing + (indexRow + 1)) - verts.at(indexCol * postSpacing + (indexRow + 1));
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;
	}

	if (isEdge[0] && isEdge[1]) {
		tmpVec[0] = verts.at((indexCol + 1) * postSpacing + (indexRow - 1)) - verts.at(indexCol * postSpacing + (indexRow - 1));
		tmpVec[1] = verts.at(indexCol * postSpacing + indexRow) - verts.at(indexCol * postSpacing + (indexRow - 1));
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;

		tmpVec[0] = verts.at((indexCol + 1) * postSpacing + (indexRow - 1)) - verts.at(indexCol * postSpacing + indexRow);
		tmpVec[1] = verts.at((indexCol + 1) * postSpacing + indexRow) - verts.at(indexCol * postSpacing + indexRow);
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;
	}

	if (isEdge[0] && isEdge[3]) {
		tmpVec[0] = verts.at(indexCol * postSpacing + (indexRow - 1)) - verts.at((indexCol - 1) * postSpacing + (indexRow - 1));
		tmpVec[1] = verts.at((indexCol - 1) * postSpacing + indexRow) - verts.at((indexCol - 1) * postSpacing + (indexRow - 1));
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;

		tmpVec[0] = verts.at(indexCol * postSpacing + (indexRow - 1)) - verts.at((indexCol - 1) * postSpacing + indexRow);
		tmpVec[1] = verts.at(indexCol * postSpacing + indexRow) - verts.at((indexCol - 1) * postSpacing + indexRow);
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;
	}

	if (isEdge[2] && isEdge[3]) {
		tmpVec[0] = verts.at(indexCol * postSpacing + indexRow) - verts.at((indexCol - 1) * postSpacing + indexRow);
		tmpVec[1] = verts.at((indexCol - 1) * postSpacing + (indexRow + 1)) - verts.at((indexCol - 1) * postSpacing + indexRow);
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;

		tmpVec[0] = verts.at(indexCol * postSpacing + indexRow) - verts.at((indexCol - 1) * postSpacing + (indexRow + 1));
		tmpVec[1] = verts.at(indexCol * postSpacing + (indexRow + 1)) - verts.at((indexCol - 1) * postSpacing + (indexRow + 1));
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;
	}

	return accumNormal / normalCount;
}

void Displacements::GenerateDispSurfNormals(const BSPStructs::DispInfo* pDispInfo, Displacement& disp)
{
	int postSpacing = ((1 << pDispInfo->power) + 1);

	for (int i = 0; i < postSpacing; i++) {
		for (int j = 0; j < postSpacing; j++) {
			bool bIsEdge[4];

			for (int k = 0; k < 4; k++) {
				bIsEdge[k] = DoesEdgeExist(j, i, k, postSpacing);
			}

			disp.normals.push_back(CalcNormalFromEdges(
				pDispInfo, disp.verts,
				j, i, bIsEdge, postSpacing
			));
		}
	}
}

void Displacements::GenerateDispSurfTangentSpaces(
	const DispInfo* pDispInfo, const Plane* pPlane,
	const TexInfo* pTexInfo,
	Displacement& disp
)
{
	Vector sAxis{ pTexInfo->textureVecs[0][0], pTexInfo->textureVecs[0][1], pTexInfo->textureVecs[0][2] };
	Vector tAxis{ pTexInfo->textureVecs[1][0], pTexInfo->textureVecs[1][1], pTexInfo->textureVecs[1][2] };

	for (int i = 0; i < disp.normals.size(); i++) {
		disp.tangents.push_back(tAxis);
		disp.tangents.at(i).Normalise();
		disp.binormals.push_back(disp.normals.at(i).Cross(disp.tangents.at(i)));
		disp.binormals.at(i).Normalise();
		disp.tangents.at(i) = disp.binormals.at(i).Cross(disp.normals.at(i));
		disp.tangents.at(i).Normalise();

		if (pPlane->normal.Dot(sAxis.Cross(tAxis)) > 0.0f) {
			disp.binormals.at(i) *= -1;
		}
	}
}
