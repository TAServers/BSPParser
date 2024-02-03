#pragma once

#include <cstdint>

namespace BSPEnums {
	enum class LUMP : uint32_t
	{
		ENTITIES,
		PLANES,
		TEXDATA,
		VERTICES,
		VISIBILITY,
		NODES,
		TEXINFO,
		FACES,
		LIGHTING,
		OCCLUSION,
		LEAFS,
		FACEIDS,
		EDGES,
		SURFEDGES,
		MODELS,
		WORLDLIGHTS,
		LEAFFACES,
		LEAFBRUSHES,
		BRUSHES,
		BRUSHSIDES,
		AREAS,
		AREAPORTALS,
		PORTALS,
		UNUSED0 = 22U,
		PROPCOLLISION = 22U,
		CLUSTERS,
		UNUSED1 = 23U,
		PROPHULLS = 23U,
		PORTALVERTS,
		UNUSED2 = 24U,
		PROPHULLVERTS = 24U,
		CLUSTERPORTALS,
		UNUSED3 = 25U,
		PROPTRIS = 25U,
		DISPINFO,
		ORIGINALFACES,
		PHYSDISP,
		PHYSCOLLIDE,
		VERTNORMALS,
		VERTNORMALINDICES,
		DISP_LIGHTMAP_ALPHAS,
		DISP_VERTS,
		DISP_LIGHTMAP_SAMPLE_POSITIONS,
		GAME_LUMP,
		LEAFWATERDATA,
		PRIMITIVES,
		PRIMVERTS,
		PRIMINDICES,
		PAKFILE,
		CLIPPORTALVERTS,
		CUBEMAPS,
		TEXDATA_STRING_DATA,
		TEXDATA_STRING_TABLE,
		OVERLAYS,
		LEAFMINDISTTOWATER,
		FACE_MACRO_TEXTURE_INFO,
		DISP_TRIS,
		PHYSCOLLIDESURFACE,
		PROP_BLOB = 49U,
		WATEROVERLAYS,
		LIGHTMAPPAGES,
		LEAF_AMBIENT_INDEX_HDR = 51U,
		LIGHTMAPPAGEINFOS,
		LEAF_AMBIENT_INDEX = 52U,
		LIGHTING_HDR,
		WORLDLIGHTS_HDR,
		LEAF_AMBIENT_LIGHTING_HDR,
		LEAF_AMBIENT_LIGHTING,
		XZIPPAKFILE,
		FACES_HDR,
		MAP_FLAGS,
		OVERLAY_FADES,
		OVERLAY_SYSTEM_LEVELS,
		PHYSLEVEL,
		DISP_MULTIBLEND,
		NONE = std::numeric_limits<uint32_t>::max(),
	};

	#define GAMELUMP_MAKE_CODE(a, b, c, d) ((a) << 24 | (b) << 16 | (c) << 8 | (d) << 0)
	enum class GameLumpID : int32_t
	{
		DETAIL_PROPS = GAMELUMP_MAKE_CODE('d', 'p', 'r', 'p'),
		DETAIL_PROP_LIGHTING = GAMELUMP_MAKE_CODE('d', 'p', 'l', 't'),
		STATIC_PROPS = GAMELUMP_MAKE_CODE('s', 'p', 'r', 'p'),
		DETAIL_PROP_LIGHTING_HDR = GAMELUMP_MAKE_CODE('d', 'p', 'l', 'h')
	};
	#undef GAMELUMP_MAKE_CODE

	enum class DetailPropOrientation : uint8_t
	{
		NORMAL = 0,
		SCREEN_ALIGNED,
		SCREEN_ALIGNED_VERTICAL
	};

	enum class DetailPropType : uint8_t
	{
		MODEL = 0,
		SPRITE,
		SHAPE_CROSS,
		SHAPE_TRI
	};

	enum class StaticPropFlags : uint8_t
	{
		// Flags field
		// These are automatically computed
		FLAG_FADES = 0x1,
		USE_LIGHTING_ORIGIN = 0x2,
		NO_DRAW = 0x4, // computed at run time based on dx level

		// These are set in WC
		IGNORE_NORMALS = 0x8,
		NO_SHADOW = 0x10,
		SCREEN_SPACE_FADE = 0x20,

		NO_PER_VERTEX_LIGHTING = 0x40, // in vrad, compute lighting at lighting origin, not for each vertex

		NO_SELF_SHADOWING = 0x80, // disable self shadowing in vrad

		WC_MASK = 0xd8 // all flags settable in hammer (?)
	};

	enum class SURF : int32_t
	{
		NONE = 0x0,
		LIGHT = 0x1,
		SKY2D = 0x2,
		SKY = 0x4,
		WARP = 0x8,
		TRANS = 0x10,
		NOPORTAL = 0x20,
		TRIGGER = 0x40,
		NODRAW = 0x80,
		HINT = 0x100,
		SKIP = 0x200,
		NOLIGHT = 0x400,
		BUMPLIGHT = 0x800,
		NOSHADOWS = 0x1000,
		NODECALS = 0x2000,
		NOCHOP = 0x4000,
		HITBOX = 0x8000
	};

	inline SURF operator |(SURF lhs, SURF rhs)
	{
		return static_cast<SURF>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
	}

	inline SURF& operator |=(SURF& lhs, SURF rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	inline SURF operator &(SURF lhs, SURF rhs)
	{
		return static_cast<SURF>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
	}

	inline SURF& operator &=(SURF& lhs, SURF rhs)
	{
		lhs = lhs & rhs;
		return lhs;
	}
}
