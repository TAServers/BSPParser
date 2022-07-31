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
	const DispInfo* pDisp, const Vector* pVerts,
	int indexRow, int indexCol, bool isEdge[4], int postSpacing
)
{
	Vector accumNormal{};
	int normalCount = 0;

	Vector tmpVec[2];
	Vector tmpNormal;

	if (isEdge[1] && isEdge[2]) {
		tmpVec[0] = pVerts[(indexCol + 1) * postSpacing + indexRow] - pVerts[indexCol * postSpacing + indexRow];
		tmpVec[1] = pVerts[indexCol * postSpacing + (indexRow + 1)] - pVerts[indexCol * postSpacing + indexRow];
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;

		tmpVec[0] = pVerts[(indexCol + 1) * postSpacing + indexRow] - pVerts[indexCol * postSpacing + (indexRow + 1)];
		tmpVec[1] = pVerts[(indexCol + 1) * postSpacing + (indexRow + 1)] - pVerts[indexCol * postSpacing + (indexRow + 1)];
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;
	}

	if (isEdge[0] && isEdge[1]) {
		tmpVec[0] = pVerts[(indexCol + 1) * postSpacing + (indexRow - 1)] - pVerts[indexCol * postSpacing + (indexRow - 1)];
		tmpVec[1] = pVerts[indexCol * postSpacing + indexRow] - pVerts[indexCol * postSpacing + (indexRow - 1)];
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;

		tmpVec[0] = pVerts[(indexCol + 1) * postSpacing + (indexRow - 1)] - pVerts[indexCol * postSpacing + indexRow];
		tmpVec[1] = pVerts[(indexCol + 1) * postSpacing + indexRow] - pVerts[indexCol * postSpacing + indexRow];
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;
	}

	if (isEdge[0] && isEdge[3]) {
		tmpVec[0] = pVerts[indexCol * postSpacing + (indexRow - 1)] - pVerts[(indexCol - 1) * postSpacing + (indexRow - 1)];
		tmpVec[1] = pVerts[(indexCol - 1) * postSpacing + indexRow] - pVerts[(indexCol - 1) * postSpacing + (indexRow - 1)];
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;

		tmpVec[0] = pVerts[indexCol * postSpacing + (indexRow - 1)] - pVerts[(indexCol - 1) * postSpacing + indexRow];
		tmpVec[1] = pVerts[indexCol * postSpacing + indexRow] - pVerts[(indexCol - 1) * postSpacing + indexRow];
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;
	}

	if (isEdge[2] && isEdge[3]) {
		tmpVec[0] = pVerts[indexCol * postSpacing + indexRow] - pVerts[(indexCol - 1) * postSpacing + indexRow];
		tmpVec[1] = pVerts[(indexCol - 1) * postSpacing + (indexRow + 1)] - pVerts[(indexCol - 1) * postSpacing + indexRow];
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;

		tmpVec[0] = pVerts[indexCol * postSpacing + indexRow] - pVerts[(indexCol - 1) * postSpacing + (indexRow + 1)];
		tmpVec[1] = pVerts[indexCol * postSpacing + (indexRow + 1)] - pVerts[(indexCol - 1) * postSpacing + (indexRow + 1)];
		tmpNormal = tmpVec[1].Cross(tmpVec[0]);
		tmpNormal.Normalise();
		accumNormal += tmpNormal;
		normalCount++;
	}

	return accumNormal / normalCount;
}

void Displacements::GenerateDispSurfNormals(
	const DispInfo* pDisp, const Vector* pVerts,
	Vector* pNormals
)
{
	int postSpacing = ((1 << pDisp->power) + 1);

	for (int i = 0; i < postSpacing; i++) {
		for (int j = 0; j < postSpacing; j++) {
			bool bIsEdge[4];

			for (int k = 0; k < 4; k++) {
				bIsEdge[k] = DoesEdgeExist(j, i, k, postSpacing);
			}

			pNormals[i * postSpacing + j] = CalcNormalFromEdges(
				pDisp, pVerts,
				j, i, bIsEdge, postSpacing
			);
		}
	}
}

void Displacements::GenerateDispSurfTangentSpaces(
	const DispInfo* pDisp, const TexInfo* pTexInfo, const Plane* pPlane,
	const Vector* pVerts, const Vector* pNormals,
	Vector* pTangents, Vector* pBinormals
)
{
	Vector sAxis{ pTexInfo->textureVecs[0][0], pTexInfo->textureVecs[0][1], pTexInfo->textureVecs[0][2] };
	Vector tAxis{ pTexInfo->textureVecs[1][0], pTexInfo->textureVecs[1][1], pTexInfo->textureVecs[1][2] };

	int postSpacing = (1 << pDisp->power) + 1;
	int size = postSpacing * postSpacing;
	for (int i = 0; i < size; i++) {
		pTangents[i] = tAxis;
		pTangents[i].Normalise();
		pBinormals[i] = pNormals[i].Cross(pTangents[i]);
		pBinormals[i].Normalise();
		pTangents[i] = pBinormals[i].Cross(pNormals[i]);
		pTangents[i].Normalise();

		if (pPlane->normal.Dot(sAxis.Cross(tAxis)) > 0.0f) {
			pBinormals[i] *= -1;
		}
	}
}
