#pragma once

#include <cstdint>

namespace BSPStructs
{
	constexpr size_t MIN_MAP_DISP_POWER = 2;
	constexpr size_t MAX_MAP_DISP_POWER = 4;

	// Max # of neighboring displacement touching a displacement's corner.
	constexpr size_t MAX_DISP_CORNER_NEIGHBORS = 4;

	#define NUM_DISP_POWER_VERTS(power)	(((1 << (power)) + 1) * ((1 << (power)) + 1))
	#define NUM_DISP_POWER_TRIS(power)	((1 << (power)) * (1 << (power)) * 2)

	constexpr size_t MAX_MAP_MODELS               = 1024;
	constexpr size_t MAX_MAP_BRUSHES              = 8192;
	constexpr size_t MAX_MAP_ENTITIES             = 8192;
	constexpr size_t MAX_MAP_TEXINFO              = 12288;
	constexpr size_t MAX_MAP_TEXDATA              = 2048;
	constexpr size_t MAX_MAP_DISPINFO             = 2048;
	constexpr size_t MAX_MAP_DISP_VERTS           = MAX_MAP_DISPINFO * ((1 << MAX_MAP_DISP_POWER) + 1) * ((1 << MAX_MAP_DISP_POWER) + 1);
	constexpr size_t MAX_MAP_DISP_TRIS            = (1 << MAX_MAP_DISP_POWER) * (1 << MAX_MAP_DISP_POWER) * 2;
	constexpr size_t MAX_DISPVERTS                = NUM_DISP_POWER_VERTS(MAX_MAP_DISP_POWER);
	constexpr size_t MAX_DISPTRIS                 = NUM_DISP_POWER_TRIS(MAX_MAP_DISP_POWER);
	constexpr size_t MAX_MAP_AREAS                = 256;
	constexpr size_t MAX_MAP_AREA_BYTES           = MAX_MAP_AREAS / 8;
	constexpr size_t MAX_MAP_AREAPORTALS          = 1024;
	constexpr size_t MAX_MAP_PLANES               = 65536;
	constexpr size_t MAX_MAP_NODES                = 65536;
	constexpr size_t MAX_MAP_BRUSHSIDES           = 65536;
	constexpr size_t MAX_MAP_LEAFS                = 65536;
	constexpr size_t MAX_MAP_VERTS                = 65536;
	constexpr size_t MAX_MAP_VERTNORMALS          = 256000;
	constexpr size_t MAX_MAP_VERTNORMALINDICES    = 256000;
	constexpr size_t MAX_MAP_FACES                = 65536;
	constexpr size_t MAX_MAP_LEAFFACES            = 65536;
	constexpr size_t MAX_MAP_LEAFBRUSHES          = 65536;
	constexpr size_t MAX_MAP_PORTALS              = 65536;
	constexpr size_t MAX_MAP_CLUSTERS             = 65536;
	constexpr size_t MAX_MAP_LEAFWATERDATA        = 32768;
	constexpr size_t MAX_MAP_PORTALVERTS          = 128000;
	constexpr size_t MAX_MAP_EDGES                = 256000;
	constexpr size_t MAX_MAP_SURFEDGES            = 512000;
	constexpr size_t MAX_MAP_LIGHTING             = 0x1000000;
	constexpr size_t MAX_MAP_VISIBILITY           = 0x1000000;
	constexpr size_t MAX_MAP_TEXTURES             = 1024;
	constexpr size_t MAX_MAP_WORLDLIGHTS          = 8192;
	constexpr size_t MAX_MAP_CUBEMAPSAMPLES       = 1024;
	constexpr size_t MAX_MAP_OVERLAYS             = 512;
	constexpr size_t MAX_MAP_WATEROVERLAYS        = 16384;
	constexpr size_t MAX_MAP_TEXDATA_STRING_DATA  = 256000;
	constexpr size_t MAX_MAP_TEXDATA_STRING_TABLE = 65536;
	constexpr size_t MAX_MAP_PRIMITIVES           = 32768;
	constexpr size_t MAX_MAP_PRIMVERTS            = 65536;
	constexpr size_t MAX_MAP_PRIMINDICES          = 65536;
}
