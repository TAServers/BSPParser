#pragma once

// Ripped straight from https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/disp_common.h#L48

#include "Displacements.h"

namespace Displacements
{
	class DispSubEdgeIterator
	{
	public:
		DispSubEdgeIterator();

		void Start(const std::vector<Displacement>& displacements, const Displacement& pDisp, int iEdge, int iSub, bool bTouchCorners = false);
		bool Next();

		const VertIndex& GetIndex() const
		{
			return mIndex;
		}
		const int GetVertIndex() const {
			return mIndex.y * ((1 << mpDisp->pInfo->power) + 1) + mIndex.x;
		}
		const int GetNBVertIndex() const {
			return mNBIndex.y * ((1 << mpNeighbor->pInfo->power) + 1) + mNBIndex.x;
		}

		bool IsLastVert() const;

	private:
		const Displacement* mpDisp;
		const Displacement* mpNeighbor;

		VertIndex mIndex;
		VertIndex mInc;

		VertIndex mNBIndex;
		VertIndex mNBInc;

		int mEnd;
		int mFreeDim;
	};
}
