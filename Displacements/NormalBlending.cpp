#include "Displacements.h"

using namespace BSPStructs;

/*void BlendCorners(DispInfo** pDispInfos, size_t numDispInfos)
{
	CUtlVector<int> nbCornerVerts;

	for (int iDisp = 0; iDisp < listSize; iDisp++) {
		CCoreDispInfo* pDisp = ppListBase[iDisp];

		int iNeighbors[512];
		int nNeighbors = GetAllNeighbors(pDisp, iNeighbors);

		// Make sure we have room for all the neighbors.
		nbCornerVerts.RemoveAll();
		nbCornerVerts.EnsureCapacity(nNeighbors);
		nbCornerVerts.AddMultipleToTail(nNeighbors);

		// For each corner.
		for (int iCorner = 0; iCorner < 4; iCorner++) {
			// Has it been touched?
			CVertIndex cornerVert = pDisp->GetCornerPointIndex(iCorner);
			int iCornerVert = pDisp->VertIndexToInt(cornerVert);
			const Vector& vCornerVert = pDisp->GetVert(iCornerVert);

			// For each displacement sharing this corner..
			Vector vAverage = pDisp->GetNormal(iCornerVert);

			for (int iNeighbor = 0; iNeighbor < nNeighbors; iNeighbor++) {
				int iNBListIndex = iNeighbors[iNeighbor];
				CCoreDispInfo* pNeighbor = ppListBase[iNBListIndex];

				// Find out which vert it is on the neighbor.
				int iNBCorner = FindNeighborCornerVert(pNeighbor, vCornerVert);
				if (iNBCorner == -1) {
					nbCornerVerts[iNeighbor] = -1; // remove this neighbor from the list.
				} else {
					CVertIndex viNBCornerVert = pNeighbor->GetCornerPointIndex(iNBCorner);
					int iNBVert = pNeighbor->VertIndexToInt(viNBCornerVert);
					nbCornerVerts[iNeighbor] = iNBVert;
					vAverage += pNeighbor->GetNormal(iNBVert);
				}
			}


			// Blend all the neighbor normals with this one.
			VectorNormalize(vAverage);
			pDisp->SetNormal(iCornerVert, vAverage);

			for (int iNeighbor = 0; iNeighbor < nNeighbors; iNeighbor++) {
				int iNBListIndex = iNeighbors[iNeighbor];
				if (nbCornerVerts[iNeighbor] == -1)
					continue;

				CCoreDispInfo* pNeighbor = ppListBase[iNBListIndex];
				pNeighbor->SetNormal(nbCornerVerts[iNeighbor], vAverage);
			}
		}
	}
}


void BlendTJuncs(DispInfo** pDispInfos, size_t numDispInfos)
{
	for (int iDisp = 0; iDisp < listSize; iDisp++) {
		CCoreDispInfo* pDisp = ppListBase[iDisp];

		for (int iEdge = 0; iEdge < 4; iEdge++) {
			CDispNeighbor* pEdge = pDisp->GetEdgeNeighbor(iEdge);

			CVertIndex viMidPoint = pDisp->GetEdgeMidPoint(iEdge);
			int iMidPoint = pDisp->VertIndexToInt(viMidPoint);

			if (pEdge->m_SubNeighbors[0].IsValid() && pEdge->m_SubNeighbors[1].IsValid()) {
				const Vector& vMidPoint = pDisp->GetVert(iMidPoint);

				CCoreDispInfo* pNeighbor1 = ppListBase[pEdge->m_SubNeighbors[0].GetNeighborIndex()];
				CCoreDispInfo* pNeighbor2 = ppListBase[pEdge->m_SubNeighbors[1].GetNeighborIndex()];

				int iNBCorners[2];
				iNBCorners[0] = FindNeighborCornerVert(pNeighbor1, vMidPoint);
				iNBCorners[1] = FindNeighborCornerVert(pNeighbor2, vMidPoint);

				if (iNBCorners[0] != -1 && iNBCorners[1] != -1) {
					CVertIndex viNBCorners[2] =
					{
						pNeighbor1->GetCornerPointIndex(iNBCorners[0]),
						pNeighbor2->GetCornerPointIndex(iNBCorners[1])
					};

					Vector vAverage = pDisp->GetNormal(iMidPoint);
					vAverage += pNeighbor1->GetNormal(viNBCorners[0]);
					vAverage += pNeighbor2->GetNormal(viNBCorners[1]);

					VectorNormalize(vAverage);
					pDisp->SetNormal(iMidPoint, vAverage);
					pNeighbor1->SetNormal(viNBCorners[0], vAverage);
					pNeighbor2->SetNormal(viNBCorners[1], vAverage);
				}
			}
		}
	}
}

void BlendEdges(DispInfo** pDispInfos, size_t numDispInfos)
{
	for (int iDisp = 0; iDisp < listSize; iDisp++) {
		CCoreDispInfo* pDisp = ppListBase[iDisp];

		for (int iEdge = 0; iEdge < 4; iEdge++) {
			CDispNeighbor* pEdge = pDisp->GetEdgeNeighbor(iEdge);

			for (int iSub = 0; iSub < 2; iSub++) {
				CDispSubNeighbor* pSub = &pEdge->m_SubNeighbors[iSub];
				if (!pSub->IsValid())
					continue;

				CCoreDispInfo* pNeighbor = ppListBase[pSub->GetNeighborIndex()];

				int iEdgeDim = g_EdgeDims[iEdge];

				CDispSubEdgeIterator it;
				it.Start(pDisp, iEdge, iSub, true);

				// Get setup on the first corner vert.
				it.Next();
				CVertIndex viPrevPos = it.GetVertIndex();

				while (it.Next()) {
					// Blend the two.
					if (!it.IsLastVert()) {
						Vector vAverage = pDisp->GetNormal(it.GetVertIndex()) + pNeighbor->GetNormal(it.GetNBVertIndex());
						VectorNormalize(vAverage);

						pDisp->SetNormal(it.GetVertIndex(), vAverage);
						pNeighbor->SetNormal(it.GetNBVertIndex(), vAverage);
					}

					// Now blend the in-between verts (if this edge is high-res).
					int iPrevPos = viPrevPos[!iEdgeDim];
					int iCurPos = it.GetVertIndex()[!iEdgeDim];

					for (int iTween = iPrevPos + 1; iTween < iCurPos; iTween++) {
						float flPercent = RemapVal(iTween, iPrevPos, iCurPos, 0, 1);
						Vector vNormal;
						VectorLerp(pDisp->GetNormal(viPrevPos), pDisp->GetNormal(it.GetVertIndex()), flPercent, vNormal);
						VectorNormalize(vNormal);

						CVertIndex viTween;
						viTween[iEdgeDim] = it.GetVertIndex()[iEdgeDim];
						viTween[!iEdgeDim] = iTween;
						pDisp->SetNormal(viTween, vNormal);
					}

					viPrevPos = it.GetVertIndex();
				}
			}
		}
	}
}*/

void Displacements::SmoothNeighboringDispSurfNormals(const BSPStructs::DispInfo** pDispInfos, size_t numDispInfos)
{
	//BlendTJuncs(pDispInfos, numDispInfos);
	//BlendCorners(pDispInfos, numDispInfos);
	//BlendEdges(pDispInfos, numDispInfos);
}
