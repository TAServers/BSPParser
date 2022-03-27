#include "BSPParser.h"
#include "FileFormat/Structs.h"

#include <cstring>
#include <stdlib.h>

// DEBUG, REMOVE
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace BSPStructs;
using namespace BSPEnums;

BSPMap::BSPMap(
	const uint8_t* pFileData, const size_t dataSize
) : mDataSize(dataSize)
{
	if (pFileData == nullptr || dataSize == 0U) return;

	mpData = reinterpret_cast<uint8_t*>(malloc(dataSize));
	memcpy(mpData, pFileData, dataSize);

	if (
		!BSPParser::ParseHeader(mpData, dataSize, &mpHeader) ||
		mpHeader->version < 19 || mpHeader->version > 21 ||
		!ParseLump(&mpEdges, &mNumEdges) ||
		!BSPParser::ParseArray(
			mpData, dataSize,
			mpHeader,
			&mpSurfEdges, &mNumSurfEdges,
			LUMP::SURFEDGES, MAX_MAP_SURFEDGES
		) ||
		!ParseLump(&mpFaces, &mNumFaces) ||
		!ParseLump(&mpTexInfos, &mNumTexInfos) ||
		!ParseLump(&mpTexData, &mNumTexDatas) ||
		!BSPParser::ParseArray(
			mpData, dataSize,
			mpHeader,
			&mpTexDataStringTable, &mNumTexDataStringTableEntries,
			LUMP::TEXDATA_STRING_TABLE, MAX_MAP_TEXDATA_STRING_TABLE
		) ||
		!BSPParser::ParseArray(
			mpData, dataSize,
			mpHeader,
			&mpTexDataStringData, &mNumTexDataStringDatas,
			LUMP::TEXDATA_STRING_DATA, MAX_MAP_TEXDATA_STRING_DATA
		)
	) {
		free(mpData);
		mpData = nullptr;
		mDataSize = 0U;
		return;
	}

	mIsValid = true;
}

BSPMap::~BSPMap()
{
	if (mpData != nullptr) free(mpData);
}

bool BSPMap::IsValid() const { return mIsValid; }

const Face* BSPMap::GetFace(size_t index) const
{
	return &mpFaces[index];
}

int main()
{
	const char* path = "C:\\Users\\eds\\Documents\\Programming\\gmodmaps\\spot_test\\spot_test.bsp";

	size_t size = std::filesystem::file_size(path);
	char* data = reinterpret_cast<char*>(malloc(size));

	auto fs = std::ifstream(path, std::ifstream::binary);
	fs.read(data, size);

	auto bsp = BSPMap(reinterpret_cast<uint8_t*>(data), size);
	free(data);

	std::cout << bsp.IsValid() << std::endl;
	if (bsp.IsValid()) {
		std::cout << bsp.GetFace(0)->texInfo;
	}

	return 0;
}
