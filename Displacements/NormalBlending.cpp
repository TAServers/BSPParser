#include "Displacements.h"
#include "SubEdgeIterator.h"

#include <vector>

using namespace Displacements;
using namespace BSPStructs;

inline float RemapVal(float val, float A, float B, float C, float D)
{
	if (A == B)
		return val >= B ? D : C;
	return C + (D - C) * (val - A) / (B - A);
}

inline void VectorLerp(const Vector& src1, const Vector& src2, float t, Vector& dest)
{
	dest.x = src1.x + (src2.x - src1.x) * t;
	dest.y = src1.y + (src2.y - src1.y) * t;
	dest.z = src1.z + (src2.z - src1.z) * t;
}

int GetAllNeighbours(const DispInfo* pDisp, int neighbors[512])
{
	int numNeighbours = 0;

	// Check corner neighbors.
	for (int iCorner = 0; iCorner < 4; iCorner++) {
		const DispCornerNeighbours* pCorner = pDisp->cornerNeighbours + iCorner;

		for (int i = 0; i < pCorner->numNeighbours; i++) {
			if (numNeighbours < 512)
				neighbors[numNeighbours++] = pCorner->neighbours[i];
		}
	}

	for (int iEdge = 0; iEdge < 4; iEdge++) {
		const DispNeighbour& edge = pDisp->edgeNeighbours[iEdge];

		for (int i = 0; i < 2; i++) {
			if (edge.subNeighbors[i].IsValid() && numNeighbours < 512)
				neighbors[numNeighbours++] = edge.subNeighbors[i].index;
		}
	}

	return numNeighbours;
}

int CornerToVertIdx(const DispInfo* pDispInfo, int corner)
{
	int sideLength = (1 << pDispInfo->power) + 1;

	int x = 0, y = 0;
	switch (corner) {
	case CORNER_UPPER_LEFT:
		y = sideLength - 1;
		break;
	case CORNER_UPPER_RIGHT:
		x = sideLength - 1;
		y = sideLength - 1;
		break;
	case CORNER_LOWER_RIGHT:
		x = sideLength - 1;
		break;
	}

	return y * sideLength + x;
}

int GetEdgeMidPoint(const DispInfo* pDispInfo, int edge)
{
	int sideLength = (1 << pDispInfo->power) + 1;

	int end = sideLength - 1;
	int mid = sideLength / 2;

	int x = 0, y = 0;
	switch (edge) {
	case NEIGHBOREDGE_LEFT:
		y = mid;
		break;
	case NEIGHBOREDGE_TOP:
		x = mid;
		y = end;
		break;
	case NEIGHBOREDGE_RIGHT:
		x = end;
		y = mid;
		break;
	case NEIGHBOREDGE_BOTTOM:
		x = mid;
		break;
	}

	return y * sideLength + x;
}

int FindNeighborCornerVert(const Displacement& disp, const Vector& vTest)
{
	int iClosest = 0;
	float flClosest = 1e24;
	for (int iCorner = 0; iCorner < 4; iCorner++) {
		int iCornerVert = CornerToVertIdx(disp.pInfo, iCorner);

		const Vector& vCornerVert = disp.verts.at(iCornerVert);

		Vector delta = vCornerVert - vTest;
		float flDist = sqrtf(delta.Dot(delta));
		if (flDist < flClosest) {
			iClosest = iCorner;
			flClosest = flDist;
		}
	}

	if (flClosest <= 0.1f)
		return iClosest;
	else
		return -1;
}

void BlendCorners(std::vector<Displacement>& displacements)
{
	std::vector<int> cornerVerts;

	for (int iDisp = 0; iDisp < displacements.size(); iDisp++) {
		Displacement& disp = displacements.at(iDisp);

		int neighbours[512];
		int numNeighbours = GetAllNeighbours(disp.pInfo, neighbours);

		cornerVerts = std::vector<int>(numNeighbours);

		for (int iCorner = 0; iCorner < 4; iCorner++) {
			int iCornerVert = CornerToVertIdx(disp.pInfo, iCorner);
			const Vector& vCornerVert = disp.verts.at(iCornerVert);

			// For each displacement sharing this corner..
			Vector average = disp.normals.at(iCornerVert);

			for (int iNeighbour = 0; iNeighbour < numNeighbours; iNeighbour++) {
				Displacement& neighbour = displacements.at(neighbours[iNeighbour]);

				// Find out which vert it is on the neighbor.
				int iNBCorner = FindNeighborCornerVert(neighbour, vCornerVert);
				if (iNBCorner == -1) {
					cornerVerts.at(iNeighbour) = -1; // remove this neighbor from the list.
				} else {
					int iNBVert = CornerToVertIdx(neighbour.pInfo, iNBCorner);
					cornerVerts.at(iNeighbour) = iNBVert;
					average += neighbour.normals.at(iNBVert);
				}
			}

			// Blend all the neighbor normals with this one.
			average.Normalise();

			disp.normals.at(iCornerVert) = average;

			for (int iNeighbour = 0; iNeighbour < numNeighbours; iNeighbour++) {
				int iNBListIndex = neighbours[iNeighbour];
				if (cornerVerts.at(iNeighbour) != -1) {
					Displacement& neighbour = displacements.at(iNBListIndex);
					neighbour.normals.at(cornerVerts.at(iNeighbour)) = average;
				}
			}
		}
	}
}

void BlendTJuncs(std::vector<Displacement>& displacements)
{
	for (int iDisp = 0; iDisp < displacements.size(); iDisp++) {
		Displacement& disp = displacements.at(iDisp);

		for (int iEdge = 0; iEdge < 4; iEdge++) {
			const DispNeighbour& edge = disp.pInfo->edgeNeighbours[iEdge];

			int iMidPoint = GetEdgeMidPoint(disp.pInfo, iEdge);

			if (edge.subNeighbors[0].IsValid() && edge.subNeighbors[1].IsValid()) {
				const Vector& vMidPoint = disp.verts.at(iMidPoint);

				Displacement& neighbour1 = displacements.at(edge.subNeighbors[0].index);
				Displacement& neighbour2 = displacements.at(edge.subNeighbors[1].index);

				int iNBCorners[2];
				iNBCorners[0] = FindNeighborCornerVert(neighbour1, vMidPoint);
				iNBCorners[1] = FindNeighborCornerVert(neighbour2, vMidPoint);

				if (iNBCorners[0] != -1 && iNBCorners[1] != -1) {
					int viNBCorners[2] =
					{
						CornerToVertIdx(neighbour1.pInfo, iNBCorners[0]),
						CornerToVertIdx(neighbour2.pInfo, iNBCorners[1])
					};

					Vector average = disp.normals.at(iMidPoint);
					average += neighbour1.normals.at(viNBCorners[0]);
					average += neighbour2.normals.at(viNBCorners[1]);

					average.Normalise();
					disp.normals.at(iMidPoint) = average;
					neighbour1.normals.at(viNBCorners[0]) = average;
					neighbour2.normals.at(viNBCorners[1]) = average;
				}
			}
		}
	}
}

void BlendEdges(std::vector<Displacement>& displacements)
{
	for (int iDisp = 0; iDisp < displacements.size(); iDisp++) {
		Displacement& disp = displacements.at(iDisp);
		int sideLength = (1 << disp.pInfo->power) + 1;

		for (int iEdge = 0; iEdge < 4; iEdge++) {
			const DispNeighbour& edge = disp.pInfo->edgeNeighbours[iEdge];

			for (int iSub = 0; iSub < 2; iSub++) {
				const DispSubNeighbour& sub = edge.subNeighbors[iSub];
				if (!sub.IsValid()) continue;

				Displacement& neighbour = displacements.at(sub.index);

				int iEdgeDim = g_EdgeDims[iEdge];

				DispSubEdgeIterator it;
				it.Start(displacements, disp, iEdge, iSub, true);

				it.Next();
				VertIndex prevPos = it.GetIndex();

				while (it.Next()) {
					if (!it.IsLastVert()) {
						Vector vAverage = disp.normals.at(it.GetVertIndex()) + neighbour.normals.at(it.GetNBVertIndex());
						vAverage.Normalise();

						disp.normals.at(it.GetVertIndex()) = vAverage;
						neighbour.normals.at(it.GetNBVertIndex()) = vAverage;
					}

					int iPrevPos = prevPos[!iEdgeDim];
					int iCurPos = it.GetIndex()[!iEdgeDim];

					for (int iTween = iPrevPos + 1; iTween < iCurPos; iTween++) {
						float flPercent = RemapVal(iTween, iPrevPos, iCurPos, 0, 1);
						Vector normal;
						VectorLerp(disp.normals.at(prevPos.y * sideLength + prevPos.x), disp.normals.at(it.GetVertIndex()), flPercent, normal);
						normal.Normalise();

						VertIndex viTween;
						viTween[iEdgeDim] = it.GetIndex()[iEdgeDim];
						viTween[!iEdgeDim] = iTween;
						disp.normals.at(viTween.y * sideLength + viTween.x) = normal;
					}

					prevPos = it.GetIndex();
				}
			}
		}
	}
}

void Displacements::SmoothNeighbouringDispSurfNormals(std::vector<Displacement>& displacements)
{
	BlendTJuncs(displacements);
	BlendCorners(displacements);
	BlendEdges(displacements);
}
