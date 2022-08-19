#pragma once

#include <cstdint>

#include "Limits.h"
#include "Enums.h"

namespace BSPStructs {
	constexpr int32_t IDBSPHEADER = 'V' + ('B' << 8) + ('S' << 16) + ('P' << 24);
	constexpr size_t HEADER_LUMPS = 64;

#pragma region Base Types
	struct Vector
	{
		float x = 0, y = 0, z = 0;

		Vector operator+(const Vector& b) const;
		Vector operator-() const;
		Vector operator-(const Vector& b) const;
		Vector operator*(const Vector& b) const;
		Vector operator/(const Vector& b) const;

		Vector& operator+=(const Vector& b);
		Vector& operator-=(const Vector& b);
		Vector& operator*=(const Vector& b);
		Vector& operator/=(const Vector& b);

		Vector operator+(const float& b) const;
		Vector operator-(const float& b) const;
		Vector operator*(const float& b) const;
		Vector operator/(const float& b) const;

		Vector& operator+=(const float& b);
		Vector& operator-=(const float& b);
		Vector& operator*=(const float& b);
		Vector& operator/=(const float& b);

		inline float Dot(const Vector& b) const
		{
			return x * b.x + y * b.y + z * b.z;
		}
		Vector Cross(const Vector& b) const;
		void Normalise();
	};

	struct Vector2D
	{
		float x = 0, y = 0;

		Vector2D operator+(const Vector2D& b) const;
		Vector2D operator-() const;
		Vector2D operator-(const Vector2D& b) const;
		Vector2D operator*(const Vector2D& b) const;
		Vector2D operator/(const Vector2D& b) const;

		Vector2D& operator+=(const Vector2D& b);
		Vector2D& operator-=(const Vector2D& b);
		Vector2D& operator*=(const Vector2D& b);
		Vector2D& operator/=(const Vector2D& b);

		Vector2D operator+(const float& b) const;
		Vector2D operator-(const float& b) const;
		Vector2D operator*(const float& b) const;
		Vector2D operator/(const float& b) const;

		Vector2D& operator+=(const float& b);
		Vector2D& operator-=(const float& b);
		Vector2D& operator*=(const float& b);
		Vector2D& operator/=(const float& b);

		inline float Dot(const Vector2D& b) const
		{
			return x * b.x + y * b.y;
		}
		void Normalise();
	};

	struct QAngle
	{
		float x = 0, y = 0, z = 0;
	};

	struct ColorRGBExp32
	{
		uint8_t r, g, b;
		int8_t exponent;
	};
#pragma endregion

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

	struct GameLump
	{
		BSPEnums::GameLumpID  id;      // gamelump ID
		uint16_t flags;   // flags
		BSPEnums::GameLumpVersion version; // gamelump version
		int32_t  offset;
		int32_t  length; // length
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

#pragma region Displacements
	struct DispSubNeighbour
	{
		uint16_t index = 0xFFFF;
		uint8_t orientation;
		uint8_t span;
		uint8_t neighbourSpan;

		inline bool IsValid() const { return index != 0xFFFF; }
	};

	struct DispNeighbour
	{
		DispSubNeighbour subNeighbors[2];
	};

	struct DispCornerNeighbours
	{
		uint16_t neighbours[MAX_DISP_CORNER_NEIGHBORS];
		uint8_t numNeighbours;
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

		DispNeighbour edgeNeighbours[4];
		DispCornerNeighbours cornerNeighbours[4];

		uint32_t allowedVerts[10];
	};

	struct DispVert
	{
		Vector vec;
		float dist;
		float alpha;
	};
#pragma endregion

#pragma region Detail Props
	struct DetailObjectDict
	{
		char modelName[DETAIL_NAME_LENGTH];
	};

	struct DetailSpriteDict
	{
		// NOTE: All detail prop sprites must lie in the material detail/detailsprites
		Vector2D upperLeft; 
		Vector2D lowerRight;
		Vector2D texUpperLeft;
		Vector2D texLowerRight;
	};

	struct DetailObject
	{
		Vector        origin;
		QAngle        angles;
		uint16_t      detailModel; // either index into DetailObjectDictLump_t or DetailPropSpriteLump_t
		uint16_t      leaf;
		ColorRGBExp32 lighting;
		uint32_t      lightStyles;
		uint8_t       lightStyleCount;
		uint8_t       swayAmount; // how much do the details sway
		uint8_t       shapeAngle; // angle param for shaped sprites
		uint8_t       shapeSize; // size param for shaped sprites
		BSPEnums::DetailPropOrientation orientation;
		uint8_t       padding2[3];
		BSPEnums::DetailPropType type;
		uint8_t       padding3[3];
		float         flScale; // For sprites only currently
	};

	struct DetailPropLightstyles
	{
		ColorRGBExp32 lighting;
		uint8_t	style;
	};
#pragma endregion

#pragma region Static Props
	struct StaticPropDict
	{
		char modelName[STATIC_PROP_NAME_LENGTH];
	};

	struct StaticPropV4
	{
		Vector   origin;
		QAngle   angles;
		uint16_t propType;
		uint16_t firstLeaf;
		uint16_t leafCount;
		uint8_t  solid;
		BSPEnums::StaticPropFlags flags;
		int32_t  skin;
		float    fadeMinDist;
		float    fadeMaxDist;
		Vector   lightingOrigin;
	};

	struct StaticPropV5
	{
		Vector   origin;
		QAngle   angles;
		uint16_t propType;
		uint16_t firstLeaf;
		uint16_t leafCount;
		uint8_t  solid;
		BSPEnums::StaticPropFlags flags;
		int32_t  skin;
		float    fadeMinDist;
		float    fadeMaxDist;
		Vector   lightingOrigin;
		float    flForcedFadeScale;
	};

	struct StaticProp
	{
		Vector   origin;
		QAngle   angles;
		uint16_t propType;
		uint16_t firstLeaf;
		uint16_t leafCount;
		uint8_t  solid;
		BSPEnums::StaticPropFlags flags;
		int32_t  skin;
		float    fadeMinDist;
		float    fadeMaxDist;
		Vector   lightingOrigin;
		float    flForcedFadeScale;
		uint16_t minDXLevel;
		uint16_t maxDXLevel;
	};

	struct StaticPropLeaf
	{
		uint16_t leaf;
	};

	struct StaticPropLightstyles
	{
		ColorRGBExp32 lighting;
	};
#pragma endregion
}
