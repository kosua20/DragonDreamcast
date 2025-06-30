#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "lodepng.h"


void loadDataFromFile(const std::string & path, std::vector<unsigned char>& data) {
	
	std::ifstream inputFile(path, std::ios::binary|std::ios::ate);
	if (inputFile.bad() || inputFile.fail()){
		return;
	}

	std::ifstream::pos_type fileSize = inputFile.tellg();
	data.resize(fileSize);

	inputFile.seekg(0, std::ios::beg);
	inputFile.read(reinterpret_cast<char*>(data.data()), data.size());
	inputFile.close();
}


int main(int argc, char** argv){
	
	if(argc < 2){ return -2; }
	std::string basename = std::string(argv[1]);

	std::string imagePath = "../textures/" + basename + ".png";
	std::string headerPath = "../../src/data/" + basename + "_texture.h";

	std::vector<unsigned char> inputData;
	loadDataFromFile(imagePath, inputData);

	std::vector<unsigned char> data;
	unsigned int w, h;

	lodepng::State state = lodepng::State();
	state.info_raw.colortype = LCT_PALETTE;
	state.info_raw.bitdepth = 8;

	lodepng::decode(data, w, h, state, inputData.data(), inputData.size());

	if(data.empty()){
		return -1;
	}

	std::cout << "Color type  : " << state.info_png.color.colortype << std::endl;
	std::cout << "Bit depth   : " << state.info_png.color.bitdepth << std::endl;
	std::cout << "Palette size: " << state.info_png.color.palettesize << std::endl;

	std::ofstream of;
	of.open(headerPath.c_str());
	if (!of) {
		std::cerr << headerPath + " is not a valid file." << std::endl;
		return 1;
	}
	const size_t size = data.size();

	of << "#ifndef __" << basename << "_texture__\n";
	of << "#define __" << basename << "_texture__\n\n";
	
//#define USE_16_BITS_COLORS
#ifdef USE_16_BITS_COLORS
	of << "unsigned short palette_" << basename << "[] __attribute__((aligned(16))) = {\n";
#else
	of << "unsigned char palette_" << basename << "[] __attribute__((aligned(16))) = {\n";
#endif
	// The layout of a palette in VRAM is 'special', split into two columns
	//  x+0  x+1  x+2  x+3  x+4  x+5  x+6  x+7    x+16 x+17 x+18 x+19 x+20 x+21 x+22 x+23
	//  x+8  x+9 x+10 x+11 x+12 x+13 x+14 x+15    x+24 x+25 x+26 x+27 x+28 x+29 x+30 x+31
	// x+32  ...

	const size_t offsets[] = {
		0, 1,  2,  3,  4,  5,  6,  7,   16, 17, 18, 19, 20, 21, 22, 23, 
		8, 9, 10, 11, 12, 13, 14, 15,   24, 25, 26, 27, 28, 29, 30, 31
	};

	for(size_t i = 0; i < state.info_png.color.palettesize; ++i){
		// Always RGBA
		const size_t baseIndex = i / 32;
		const size_t offsetIndex = offsets[i % 32];
		const size_t finalIndex = baseIndex * 32 + offsetIndex;

		unsigned char* color = &state.info_png.color.palette[4 * finalIndex];

#ifdef USE_16_BITS_COLORS
		unsigned char r = color[0] >> 3u;
		unsigned char g = color[1] >> 3u;
		unsigned char b = color[2] >> 3u;
		unsigned char a = 1;

		of << int( (a << 15u) | (b << 10u) | (g << 5u) | (r << 0u)) << ", ";
#else
		of << int(color[0]) << "," << int(color[1]) << "," << int(color[2]) << "," << int(color[3]) << ", ";
#endif
		of << "\n";
	}
	for(size_t i = state.info_png.color.palettesize; i < 256; ++i){
		of << "0,0,0,255, ";
	}
	of << "};\n\n";

	of << "unsigned char texture_" << basename << "[] __attribute__((aligned(16))) = {\n";
	for(size_t y = 0; y < h; ++y){
		for(size_t x = 0; x < w; ++x){
			of << int(data[y*w + x]) << ", ";
		}
		of << "\n";
	}
	
	of << "};\n";

	of << "#endif\n";
	
	
	of.close();

	//stbi_image_free(data);
	return 0;
}
