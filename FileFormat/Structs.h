#pragma once

#include <cstdint>

#include "Limits.h"
#include "Enums.h"

namespace BSPStructs {
	constexpr int32_t IDBSPHEADER = 'V' + ('B' << 8) + ('S' << 16) + ('P' << 24);
	constexpr size_t HEADER_LUMPS = 64;

	struct Lump
	{
		int32_t offset; // Byte offset into file
		int32_t length; // Length of lump data
		int32_t version; // Lump format version
		int32_t fourCC; // Uncompressed size, or 0
	};

	struct Header
	{
		int32_t ident; // BSP file identifier (should always equal "VBSP")
		int32_t version; // Version of the BSP file (19-21)
		Lump lumps[HEADER_LUMPS]; // Lump directory
		int32_t mapRevision; // Map version number
	};

	struct Vector
	{
		float x, y, z;
	};

	struct Plane
	{
		Vector normal; // Normal of the plane
		float distance; // Distance from origin
		int32_t type; // Plane axis identifier (unused)
	};

	struct Edge
	{
		uint16_t vertices[2]; // Indices of each vertex that makes up this edge
	};

	struct Face
	{
		uint16_t planeNum;
		uint8_t side;
		uint8_t onNode;
		int32_t firstEdge;
		int16_t numEdges;
		int16_t texInfo;
		int16_t dispInfo;
		int16_t surfaceVolumeFogId;
		uint8_t styles[4];
		int32_t lightOffset;
		float area; // Area of the face in hammer units squared
		int32_t lightmapTextureMinsInLuxels[2];
		int32_t lightmapTextureSizeInLuxels[2];
		int32_t originalFace;
		uint16_t numPrimitives;
		uint16_t firstPrimitiveId;
		uint32_t smoothingGroups;
	};

	struct Brush
	{
		int32_t firstSide;
		int32_t numSides;
		int32_t contents;
	};

	struct BrushSide
	{
		uint16_t planeNum;
		int16_t texInfo;
		int16_t dispInfo;
		int16_t bevel;
	};

	struct TexInfo
	{
		float textureVecs[2][4];
		float lightmapVecs[2][4];
		BSPEnums::SURF flags;
		int32_t texData;
	};

	struct TexData
	{
		Vector reflectivity;
		int32_t nameStringTableId;
		int32_t width, height;
		int32_t viewWidth, viewHeight;
	};

	struct Model
	{
		Vector mins, maxs;
		Vector origin;
		int32_t headNode;
		int32_t firstFace, numFaces;
	};

	struct DispInfo
	{
		Vector startPosition;
	
		int32_t dispVertStart;
		int32_t dispTriStart;
	
		int32_t power;
		int32_t minTess;
	
		float smoothingAngle;
		int32_t contents;
		uint16_t mapFace;
		int32_t lightmapAlphaStart;
		int32_t lightmapSamplePositionStart;

		// CDispNeighbour edgeNeighbours[4]
		// CDispCornerNeighbours cornerNeighbours[4]
		uint8_t unused[88];

		uint32_t allowedVerts[10];
	};

	struct DispVert
	{
		Vector vec;
		float dist;
		float alpha;
	};
}
