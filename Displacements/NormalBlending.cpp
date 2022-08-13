#include "Displacements.h"
#include "SubEdgeIterator.h"

#include <vector>
#include <cmath>

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

		const Vector& vCornerVert = disp.verts[iCornerVert];

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
		Displacement& disp = displacements[iDisp];

		int neighbours[512];
		int numNeighbours = GetAllNeighbours(disp.pInfo, neighbours);

		cornerVerts = std::vector<int>(numNeighbours);

		for (int iCorner = 0; iCorner < 4; iCorner++) {
			int iCornerVert = CornerToVertIdx(disp.pInfo, iCorner);
			const Vector& vCornerVert = disp.verts[iCornerVert];

			// For each displacement sharing this corner..
			int divisor = 1;
			Vector averageT = disp.tangents[iCornerVert];
			Vector averageB = disp.binormals[iCornerVert];
			Vector averageN = disp.normals[iCornerVert];

			for (int iNeighbour = 0; iNeighbour < numNeighbours; iNeighbour++) {
				Displacement& neighbour = displacements[neighbours[iNeighbour]];

				// Find out which vert it is on the neighbor.
				int iNBCorner = FindNeighborCornerVert(neighbour, vCornerVert);
				if (iNBCorner == -1) {
					cornerVerts[iNeighbour] = -1; // remove this neighbor from the list.
				} else {
					int iNBVert = CornerToVertIdx(neighbour.pInfo, iNBCorner);
					cornerVerts[iNeighbour] = iNBVert;

					averageT += neighbour.tangents[iNBVert];
					averageB += neighbour.binormals[iNBVert];
					averageN += neighbour.normals[iNBVert];
					divisor++;
				}
			}

			// Blend all the neighbor normals with this one.
			averageT /= divisor;
			averageB /= divisor;
			averageN /= divisor;

			disp.tangents[iCornerVert] = averageT;
			disp.binormals[iCornerVert] = averageB;
			disp.normals[iCornerVert] = averageN;

			for (int iNeighbour = 0; iNeighbour < numNeighbours; iNeighbour++) {
				int iNBListIndex = neighbours[iNeighbour];
				if (cornerVerts[iNeighbour] != -1) {
					Displacement& neighbour = displacements[iNBListIndex];

					neighbour.tangents[cornerVerts[iNeighbour]] = averageT;
					neighbour.binormals[cornerVerts[iNeighbour]] = averageB;
					neighbour.normals[cornerVerts[iNeighbour]] = averageN;
				}
			}
		}
	}
}

void BlendTJuncs(std::vector<Displacement>& displacements)
{
	for (int iDisp = 0; iDisp < displacements.size(); iDisp++) {
		Displacement& disp = displacements[iDisp];

		for (int iEdge = 0; iEdge < 4; iEdge++) {
			const DispNeighbour& edge = disp.pInfo->edgeNeighbours[iEdge];

			int iMidPoint = GetEdgeMidPoint(disp.pInfo, iEdge);

			if (edge.subNeighbors[0].IsValid() && edge.subNeighbors[1].IsValid()) {
				const Vector& vMidPoint = disp.verts[iMidPoint];

				Displacement& neighbour1 = displacements[edge.subNeighbors[0].index];
				Displacement& neighbour2 = displacements[edge.subNeighbors[1].index];

				int iNBCorners[2];
				iNBCorners[0] = FindNeighborCornerVert(neighbour1, vMidPoint);
				iNBCorners[1] = FindNeighborCornerVert(neighbour2, vMidPoint);

				if (iNBCorners[0] != -1 && iNBCorners[1] != -1) {
					int viNBCorners[2] =
					{
						CornerToVertIdx(neighbour1.pInfo, iNBCorners[0]),
						CornerToVertIdx(neighbour2.pInfo, iNBCorners[1])
					};

					Vector averageT = disp.tangents[iMidPoint];
					Vector averageB = disp.binormals[iMidPoint];
					Vector averageN = disp.normals[iMidPoint];

					averageT += neighbour1.tangents[viNBCorners[0]] + neighbour2.tangents[viNBCorners[1]];
					averageB += neighbour1.binormals[viNBCorners[0]] + neighbour2.binormals[viNBCorners[1]];
					averageN += neighbour1.normals[viNBCorners[0]] + neighbour2.normals[viNBCorners[1]];

					averageT /= 3;
					averageB /= 3;
					averageN /= 3;

					disp.tangents[iMidPoint] = neighbour1.tangents[viNBCorners[0]] = neighbour2.tangents[viNBCorners[1]] = averageT;
					disp.binormals[iMidPoint] = neighbour1.binormals[viNBCorners[0]] = neighbour2.binormals[viNBCorners[1]] = averageB;
					disp.normals[iMidPoint] = neighbour1.normals[viNBCorners[0]] = neighbour2.normals[viNBCorners[1]] = averageN;
				}
			}
		}
	}
}

void BlendEdges(std::vector<Displacement>& displacements)
{
	for (int iDisp = 0; iDisp < displacements.size(); iDisp++) {
		Displacement& disp = displacements[iDisp];
		int sideLength = (1 << disp.pInfo->power) + 1;

		for (int iEdge = 0; iEdge < 4; iEdge++) {
			const DispNeighbour& edge = disp.pInfo->edgeNeighbours[iEdge];

			for (int iSub = 0; iSub < 2; iSub++) {
				const DispSubNeighbour& sub = edge.subNeighbors[iSub];
				if (!sub.IsValid()) continue;

				Displacement& neighbour = displacements[sub.index];

				int iEdgeDim = g_EdgeDims[iEdge];

				DispSubEdgeIterator it;
				it.Start(displacements, disp, iEdge, iSub, true);

				it.Next();
				VertIndex prevPos = it.GetIndex();

				while (it.Next()) {
					if (!it.IsLastVert()) {
						Vector averageT = disp.tangents[it.GetVertIndex()] + neighbour.tangents[it.GetNBVertIndex()];
						Vector averageB = disp.binormals[it.GetVertIndex()] + neighbour.binormals[it.GetNBVertIndex()];
						Vector averageN = disp.normals[it.GetVertIndex()] + neighbour.normals[it.GetNBVertIndex()];
						averageT /= 2;
						averageB /= 2;
						averageN /= 2;

						disp.tangents[it.GetVertIndex()] = averageT;
						disp.binormals[it.GetVertIndex()] = averageB;
						disp.normals[it.GetVertIndex()] = averageN;
						neighbour.tangents[it.GetNBVertIndex()] = averageT;
						neighbour.binormals[it.GetNBVertIndex()] = averageB;
						neighbour.normals[it.GetNBVertIndex()] = averageN;
					}

					int iPrevPos = prevPos[!iEdgeDim];
					int iCurPos = it.GetIndex()[!iEdgeDim];

					for (int iTween = iPrevPos + 1; iTween < iCurPos; iTween++) {
						float flPercent = RemapVal(iTween, iPrevPos, iCurPos, 0, 1);

						int coord = prevPos.y * sideLength + prevPos.x;
						Vector tangent, binormal, normal;

						VectorLerp(disp.tangents[coord], disp.tangents[it.GetVertIndex()], flPercent, tangent);
						VectorLerp(disp.binormals[coord], disp.binormals[it.GetVertIndex()], flPercent, binormal);
						VectorLerp(disp.normals[coord], disp.normals[it.GetVertIndex()], flPercent, normal);

						tangent.Normalise();
						binormal.Normalise();
						normal.Normalise();

						VertIndex viTween;
						viTween[iEdgeDim] = it.GetIndex()[iEdgeDim];
						viTween[!iEdgeDim] = iTween;
						coord = viTween.y * sideLength + viTween.x;

						disp.tangents[coord] = tangent;
						disp.binormals[coord] = binormal;
						disp.normals[coord] = normal;
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
