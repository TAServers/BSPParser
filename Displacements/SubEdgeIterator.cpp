#include "SubEdgeIterator.h"

#include <vector>
#include <stdexcept>

using namespace Displacements;
using namespace BSPStructs;

constexpr unsigned char CORNER_TO_CORNER   = 0;
constexpr unsigned char CORNER_TO_MIDPOINT = 1;
constexpr unsigned char MIDPOINT_TO_CORNER = 2;

constexpr unsigned char ORIENTATION_CCW_0 = 0;
constexpr unsigned char ORIENTATION_CCW_90 = 1;
constexpr unsigned char ORIENTATION_CCW_180 = 2;
constexpr unsigned char ORIENTATION_CCW_270 = 3;

int g_EdgeSideLenMul[4] =
{
	0,
	1,
	1,
	0
};

struct ShiftInfo
{
	int  midPointScale;
	int  powerShiftAdd;
	bool valid;
};
ShiftInfo g_ShiftInfos[3][3] =
{
	{
		{0,  0, true},		// CORNER_TO_CORNER -> CORNER_TO_CORNER
		{0, -1, true},		// CORNER_TO_CORNER -> CORNER_TO_MIDPOINT
		{2, -1, true}		// CORNER_TO_CORNER -> MIDPOINT_TO_CORNER
	},
	{
		{0,  1, true},		// CORNER_TO_MIDPOINT -> CORNER_TO_CORNER
		{0,  0, false},		// CORNER_TO_MIDPOINT -> CORNER_TO_MIDPOINT (invalid)
		{0,  0, false}		// CORNER_TO_MIDPOINT -> MIDPOINT_TO_CORNER (invalid)
	},
	{
		{-1, 1, true},		// MIDPOINT_TO_CORNER -> CORNER_TO_CORNER
		{0,  0, false},		// MIDPOINT_TO_CORNER -> CORNER_TO_MIDPOINT (invalid)
		{0,  0, false}		// MIDPOINT_TO_CORNER -> MIDPOINT_TO_CORNER (invalid)
	}
};

VertIndex GetCornerPointIndex(int power, int corner)
{
	int sideLengthM1 = 1 << power;

	VertIndex v{ 0, 0 };
	switch (corner) {
	case CORNER_UPPER_LEFT:
		v.y = sideLengthM1;
		break;
	case CORNER_UPPER_RIGHT:
		v.x = sideLengthM1;
		v.y = sideLengthM1;
		break;
	case CORNER_LOWER_RIGHT:
		v.x = sideLengthM1;
		break;
	}

	return v;
}

void SetupSpan(int power, int iEdge, unsigned char span, VertIndex& viStart, VertIndex& viEnd)
{
	int iFreeDim = !g_EdgeDims[iEdge];

	viStart = GetCornerPointIndex(power, iEdge);
	viEnd = GetCornerPointIndex(power, (iEdge + 1) & 3);

	if (iEdge == NEIGHBOREDGE_RIGHT || iEdge == NEIGHBOREDGE_BOTTOM) {
		if (span == CORNER_TO_MIDPOINT)
			viStart[iFreeDim] = ((1 << power) + 1) / 2;
		else if (span == MIDPOINT_TO_CORNER)
			viEnd[iFreeDim] = ((1 << power) + 1) / 2;
	} else {
		if (span == CORNER_TO_MIDPOINT)
			viEnd[iFreeDim] = ((1 << power) + 1) / 2;
		else if (span == MIDPOINT_TO_CORNER)
			viStart[iFreeDim] = ((1 << power) + 1) / 2;
	}
}

void TransformIntoSubNeighbor(
	const std::vector<Displacement>& displacements,
	const Displacement& disp,
	int iEdge,
	int iSub,
	const VertIndex& nodeIndex,
	VertIndex& out
)
{
	const DispSubNeighbour& sub = disp.pInfo->edgeNeighbours[iEdge].subNeighbors[iSub];

	// Find the part of pDisp's edge that this neighbor covers.
	VertIndex viSrcStart, viSrcEnd;
	SetupSpan(disp.pInfo->power, iEdge, sub.span, viSrcStart, viSrcEnd);

	// Find the corresponding parts on the neighbor.
	const Displacement& neighbour = displacements.at(sub.index);
	int iNBEdge = (iEdge + 2 + sub.orientation) & 3;

	VertIndex viDestStart, viDestEnd;
	SetupSpan(neighbour.pInfo->power, iNBEdge, sub.span, viDestEnd, viDestStart);

	// Now map the one into the other.
	int iFreeDim = !g_EdgeDims[iEdge];
	int fixedPercent = ((nodeIndex[iFreeDim] - viSrcStart[iFreeDim]) * (1 << 16)) / (viSrcEnd[iFreeDim] - viSrcStart[iFreeDim]);
	if (fixedPercent < 0 || fixedPercent > (1 << 16)) throw std::out_of_range("fixedPercent out of range");

	int nbDim = g_EdgeDims[iNBEdge];
	out[nbDim] = viDestStart[nbDim];
	out[!nbDim] = viDestStart[!nbDim] + ((viDestEnd[!nbDim] - viDestStart[!nbDim]) * fixedPercent) / (1 << 16);

	int nbSideLen = (1 << neighbour.pInfo->power) + 1;
	if (out.x < 0 || out.x >= nbSideLen) throw std::out_of_range("x out of bounds in neighbour");
	if (out.y < 0 || out.y >= nbSideLen) throw std::out_of_range("y out of bounds in neighbour");
}

static inline void RotateVertIncrement(
	unsigned char neighor,
	const VertIndex& in,
	VertIndex& out)
{
	if (neighor == ORIENTATION_CCW_0) {
		out = in;
	} else if (neighor == ORIENTATION_CCW_90) {
		out.x = in.y;
		out.y = -in.x;
	} else if (neighor == ORIENTATION_CCW_180) {
		out.x = -in.x;
		out.y = -in.y;
	} else {
		out.x = -in.y;
		out.y = in.x;
	}
}

const Displacement* SetupEdgeIncrements(
	const std::vector<Displacement>& displacements,
	const Displacement& disp,
	int iEdge,
	int iSub,
	VertIndex& myIndex,
	VertIndex& myInc,
	VertIndex& nbIndex,
	VertIndex& nbInc,
	int& myEnd,
	int& iFreeDim
)
{
	int iEdgeDim = g_EdgeDims[iEdge];
	iFreeDim = !iEdgeDim;

	const DispSubNeighbour& sub = disp.pInfo->edgeNeighbours[iEdge].subNeighbors[iSub];
	if (!sub.IsValid()) return nullptr;

	const Displacement& neighbour = displacements.at(sub.index);

	// Using the shift info actually causes indexing out of range into the neighbour's verts
	// I have tried figuring out how Source doesn't encounter this issue to no avail
	// Removing the shift add below seems to have no effect on the final normals however (in fact they appear smoother)
	// So my only conclusion is that Source literally causes heap corruption in the displacement deserialization inside VRAD
	// If anyone has a better explanation please raise an issue on GitHub
	//const ShiftInfo& shiftInfo = g_ShiftInfos[sub.span][sub.neighbourSpan];
	//if (!shiftInfo.valid) throw std::runtime_error("Shift info invalid");

	VertIndex tempInc;

	int sideLength = (1 << disp.pInfo->power) + 1;
	myIndex[iEdgeDim] = g_EdgeSideLenMul[iEdge] * (sideLength - 1);
	myIndex[iFreeDim] = (sideLength / 2) * iSub;
	TransformIntoSubNeighbor(displacements, disp, iEdge, iSub, myIndex, nbIndex);

	int myPower = disp.pInfo->power;
	int nbPower = neighbour.pInfo->power;// + shiftInfo.powerShiftAdd;

	myInc[iEdgeDim] = tempInc[iEdgeDim] = 0;
	if (nbPower > myPower) {
		myInc[iFreeDim] = 1;
		tempInc[iFreeDim] = 1 << (nbPower - myPower);
	} else {
		myInc[iFreeDim] = 1 << (myPower - nbPower);
		tempInc[iFreeDim] = 1;
	}
	RotateVertIncrement(sub.orientation, tempInc, nbInc);

	if (sub.span == CORNER_TO_MIDPOINT)
		myEnd = sideLength >> 1;
	else
		myEnd = sideLength - 1;

	return &neighbour;
}

DispSubEdgeIterator::DispSubEdgeIterator()
{
	mpDisp = nullptr;
	mpNeighbor = nullptr;
	mFreeDim = mIndex.x = mInc.x = mEnd = 0;
}

void DispSubEdgeIterator::Start(const std::vector<Displacement>& displacements, const Displacement& disp, int iEdge, int iSub, bool bTouchCorners)
{
	mpDisp = &disp;
	mpNeighbor = SetupEdgeIncrements(displacements, disp, iEdge, iSub, mIndex, mInc, mNBIndex, mNBInc, mEnd, mFreeDim);
	if (mpNeighbor) {
		if (bTouchCorners) {
			mIndex.x -= mInc.x;
			mIndex.y -= mInc.y;
			mNBIndex.x -= mNBInc.x;
			mNBIndex.y -= mNBInc.y;

			mEnd += mInc[mFreeDim];
		}
	} else {
		mFreeDim = mIndex.x = mInc.x = mEnd = 0;
	}
}

bool DispSubEdgeIterator::Next()
{
	mIndex.x += mInc.x;
	mIndex.y += mInc.y;
	mNBIndex.x += mNBInc.x;
	mNBIndex.y += mNBInc.y;

	return mIndex[mFreeDim] < mEnd;
}

bool DispSubEdgeIterator::IsLastVert() const
{
	return (mIndex[mFreeDim] + mInc[mFreeDim]) >= mEnd;
}
