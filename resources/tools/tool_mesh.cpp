#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <iomanip>
#include <cstdint>

typedef uint32_t u32;
typedef short int v16;
typedef short t16;


typedef struct {
	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<float> texcoords;
	std::vector<unsigned int> indices;
	std::vector<unsigned int> splits;
} mesh_t;



void loadObj(const std::string & filename, mesh_t & mesh){
	// Open the file.
	std::ifstream in;
	in.open(filename.c_str());
	if (!in) {
		std::cerr << filename + " is not a valid file." << std::endl;
		return;
	}
	//Init the mesh.
	mesh.indices.clear();
	mesh.positions.clear();
	mesh.normals.clear();
	mesh.texcoords.clear();
	// Init temporary vectors.
	std::vector<float> positions_temp;
	std::vector<float> normals_temp;
	std::vector<float> texcoords_temp;
	std::vector<std::string> faces_temp;
	std::vector<unsigned int> splits_temp;
	
	std::string res;
	
	// Iterate over the lines of the file.
	while(!in.eof()){
		getline(in,res);
		
		// Ignore the line if it is too short or a comment.
		if(res.empty() || res[0] == '#' || res.size()<2){
			continue;
		}
		//We want to split the content of the line at spaces, use a stringstream directly
		std::stringstream ss(res);
		std::vector<std::string> tokens;
		std::string token;
		while(ss >> token){
			tokens.push_back(token);
		}
		if(tokens.size() < 1){
			continue;
		}
		// Check what kind of element the line represent.
		if (tokens[0] == "v") { // Vertex position
			// We need 3 coordinates.
			if(tokens.size() < 4){
				continue;
			}
			//glm::vec3 pos = glm::vec3(,stof(tokens[2],NULL),stof(tokens[3],NULL));
			positions_temp.push_back(stof(tokens[1],NULL));
			positions_temp.push_back(stof(tokens[2],NULL));
			positions_temp.push_back(stof(tokens[3],NULL));
			
		} else if (tokens[0] == "vn"){ // Vertex normal
			// We need 3 coordinates.
			if(tokens.size() < 4){
				continue;
			}
			//glm::vec3 nor = glm::vec3(,stof(tokens[2],NULL),stof(tokens[3],NULL));
			normals_temp.push_back(stof(tokens[1],NULL));
			normals_temp.push_back(stof(tokens[2],NULL));
			normals_temp.push_back(stof(tokens[3],NULL));
			
		} else if (tokens[0] == "vt") { // Vertex UV
			// We need 2 coordinates.
			if(tokens.size() < 3){
				continue;
			}
			//glm::vec2 uv = glm::vec2(stof(tokens[1],NULL),stof(tokens[2],NULL));
			texcoords_temp.push_back(stof(tokens[1],NULL));
			texcoords_temp.push_back(stof(tokens[2],NULL));
			
		} else if (tokens[0] == "f") { // Face indices.
			// We need 3 elements, each containing at most three indices.
			if(tokens.size() < 4){
				continue;
			}
			faces_temp.push_back(tokens[1]);
			faces_temp.push_back(tokens[2]);
			faces_temp.push_back(tokens[3]);
		} else if(tokens[0] == "s"){
			splits_temp.push_back((unsigned int)faces_temp.size());
		} else { // Ignore s, l, g, matl or others
			continue;
		}
	}
	in.close();
	
	// If no vertices, end.
	if(positions_temp.size() == 0){
		return;
	}
	
	// Does the mesh have UV or normal coordinates ?
	bool hasUV = texcoords_temp.size()>0;
	bool hasNormals = normals_temp.size()>0;
	
	mesh.indices.clear();
	mesh.positions.clear();
	mesh.normals.clear();
	mesh.texcoords.clear();

	// Mode: Indexed
	// In this mode, vertices are only duplicated if they were already used in a previous face with a different set of uv/normal coordinates.

	// Keep track of previously encountered (position,uv,normal).
	std::map<std::string,unsigned int> indices_used;

	//Positions
	unsigned int maxInd = 0;
	for(size_t i = 0; i < faces_temp.size(); i++){
		
		std::string str = faces_temp[i];
		
		//Does the association of attributs already exists ?
		if(indices_used.count(str)>0){
			// Just store the index in the indices vector.
			mesh.indices.push_back(indices_used[str]);
			// Go to next face.
			continue;
		}
		
		// else, query the associated position/uv/normal, store it, update the indices vector and the list of used elements.
		size_t foundF = str.find_first_of("/");
		size_t foundL = str.find_last_of("/");
		
		//Positions (we are sure they exist)
		unsigned int ind1 = stoi(str.substr(0,foundF))-1;
		mesh.positions.push_back(positions_temp[3*ind1]);
		mesh.positions.push_back(positions_temp[3*ind1+1]);
		mesh.positions.push_back(positions_temp[3*ind1+2]);
		
		//UVs (second index)
		if(hasUV){
			unsigned int ind2 = stoi(str.substr(foundF+1,foundL))-1;
			mesh.texcoords.push_back(texcoords_temp[2*ind2]);
			mesh.texcoords.push_back(texcoords_temp[2*ind2+1]);
		}
		//Normals (third index, in all cases)
		if(hasNormals){
			unsigned int ind3 = stoi(str.substr(foundL+1))-1;
			mesh.normals.push_back(normals_temp[3*ind3]);
			mesh.normals.push_back(normals_temp[3*ind3+1]);
			mesh.normals.push_back(normals_temp[3*ind3+2]);
		}
		
		mesh.indices.push_back(maxInd);
		indices_used[str] = maxInd;
		maxInd++;
	}
	indices_used.clear();

	// Splits are copied as-is
	mesh.splits = splits_temp;
	if(mesh.splits.empty()){
		mesh.splits.push_back(0);
	}
	/*
	// Mode: Expanded
	// In this mode, vertices are all duplicated. Each face has its set of 3 vertices, not shared with any other face.
	
	// For each face, query the needed positions, normals and uvs, and add them to the mesh structure.
	for(size_t i = 0; i < faces_temp.size(); i++){
		std::string str = faces_temp[i];
		size_t foundF = str.find_first_of("/");
		size_t foundL = str.find_last_of("/");
		
		// Positions (we are sure they exist).
		long ind1 = stol(str.substr(0,foundF))-1;
		mesh.positions.push_back(positions_temp[3*ind1]);
		mesh.positions.push_back(positions_temp[3*ind1+1]);
		mesh.positions.push_back(positions_temp[3*ind1+2]);
		
		// UVs (second index).
		if(hasUV){
			long ind2 = stol(str.substr(foundF+1,foundL))-1;
			mesh.texcoords.push_back(texcoords_temp[2*ind2]);
			mesh.texcoords.push_back(texcoords_temp[2*ind2+1]);
		}
		
		// Normals (third index, in all cases).
		if(hasNormals){
			long ind3 = stol(str.substr(foundL+1))-1;
			mesh.normals.push_back(normals_temp[3*ind3]);
			mesh.normals.push_back(normals_temp[3*ind3+1]);
			mesh.normals.push_back(normals_temp[3*ind3+2]);
		}
		
		//Indices (simply a vector of increasing integers).
		mesh.indices.push_back(i);
	}
	*/
	
	std::cout << "OBJ: loaded. " << mesh.indices.size()/3 << " faces, " << mesh.positions.size()/3 << " vertices, " << mesh.normals.size()/3 << " normals, " << mesh.texcoords.size()/2 << " texcoords." << std::endl;
	return;
}



int main(int argc, char** argv){
	
	if(argc < 3){ return -2; }
	std::string basename = std::string(argv[1]);
	float scale = std::stof(argv[2]);
	
	mesh_t mesh;
	loadObj(basename+".obj", mesh);
	std::cout << "Loaded." << std::endl;
	
	std::ofstream of;
	std::string filename = basename + "_data.h";
	of.open(filename.c_str());
	if (!of) {
		std::cerr << filename + " is not a valid file." << std::endl;
		return 1;
	}
	
	int points_count = mesh.indices.size();
	int vertices_count = mesh.positions.size()/3;
	int splits_count = mesh.splits.size();


	if(!mesh.texcoords.empty()){
		if(mesh.positions.size()/3 != mesh.texcoords.size()/2){
			return -1;
		}
	}
	if(!mesh.normals.empty()){
		if(mesh.positions.size()/3 != mesh.normals.size()/3){
			return -1;
		}
	}

	
	if(vertices_count > 0xFFFF){
		return -1;
	}
	if(points_count > 0xFFFF){
		return -1;
	}
	if(splits_count > 0xFFFF){
		return -1;
	}

	of << "const uint16_t points_count_" << basename << " = " << points_count << ";" << std::endl << std::endl;
	of << "const uint16_t vertex_count_" << basename << " = " << vertices_count << ";" << std::endl << std::endl;
	of << "const uint16_t splits_count_" << basename << " = " << splits_count << ";" << std::endl << std::endl;
	
	const char* attribute = " __attribute__((aligned(32)))";

	of << "uint16_t points_" << basename << "[" << points_count << "] " << attribute << "= {" << std::endl;
	for(int i = 0; i < points_count; i+=3){
		if(i%3 == 0){
			of << "\t";
		}
		of << mesh.indices[i];
		if(i != points_count - 1){
			of << ", ";
		}
		if(i%3 == 2){
			of << std::endl;
		}
		of << mesh.indices[i+1] << ", " << mesh.indices[i+2] << (i+2 == points_count - 1 ? "" : ", ") << std::endl;
	}
	of << "};" << std::endl << std::endl;

	of << "uint16_t splits_" << basename << "[" << splits_count << "] " << attribute << "= {" << std::endl;
	for(int i = 0; i < splits_count; ++i){
		of << mesh.splits[i];
		if(i != splits_count - 1){
			of << ", ";
		}
	}
	of << "};" << std::endl << std::endl;

	
	of << std::fixed << std::setprecision(4);
	
	of << "vec3f_t vertices_" << basename << "[" << vertices_count << "] " << attribute << "= {" << std::endl;
	for(int i = 0; i < mesh.positions.size(); i+=3){
		of << "\t{.x= " << -scale*mesh.positions[i] << ", .y=" << scale*mesh.positions[i+1] <<  ", .z=" << scale*mesh.positions[i+2] <<"},\n";
		
	}
	of << "};" << std::endl << std::endl;

	if(!mesh.normals.empty()){
		of << "vec3f_t normals_" << basename << "[" << vertices_count << "] = {" << std::endl;
		for(int i = 0; i < mesh.normals.size(); i+=3){
			of << "\t{.x=" << -mesh.normals[i] << ", .y=" << mesh.normals[i+1] <<  ", .z=" << mesh.normals[i+2] << "},\n";
			
		}
		of << "};" << std::endl << std::endl;
	}

	if(!mesh.texcoords.empty()){
		of << "float texcoords_" << basename << "[" << 2*vertices_count << "] = {" << std::endl;
		for(int i = 0; i < mesh.texcoords.size(); i+=2){
			of << "\t" << mesh.texcoords[i] << ", " << (1.0f-mesh.texcoords[i+1]) << ",\n";
			
		}
		of << "};" << std::endl << std::endl;
	}
	
	
	of.close();
	return 0;
}
