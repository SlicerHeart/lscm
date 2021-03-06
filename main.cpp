// Least squares conformal mapping
//
// Author   : Mi, Liang (Arizona State University)
// Email    : icemiliang@gmail.com
// Date     : June 13th 2018

#include <cmath>
#include "Mesh.h"
#include "FormTrait.h"
#include "LSCM.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string.h>

using namespace MeshLib;

int main(int argc, char * argv[])
{
	// Parse the fixed vertex definitions if any
	// They are give in format --fixed-vertex coordX,coordY,coordZ or -v coordX,coordY,coordZ
	int currentArgument = 1;
	std::string inFilePath("");
	std::string outFilePath("");
	std::vector<FixedVertexDefinition> fixedVertices;
	while (currentArgument < argc) {
		// Fixed point positions: --fixedPoint or -p
		if (!strcmp(argv[currentArgument], "--fixedPoint") || !strcmp(argv[currentArgument], "-p")) {
			currentArgument++;
			std::string fixedVertexArg(argv[currentArgument]);
			double fixedVertexPos[3] = {0.0, 0.0, 0.0};
			for (int i=0; i<3; i++) {
				size_t commaPos = fixedVertexArg.find(",");
				if (i<2 && commaPos == std::string::npos) {
					std::cerr << "Invalid fixed vertex argument: " << argv[currentArgument] << std::endl;
					break;
				}
				std::stringstream ss;
				ss << fixedVertexArg.substr(0, commaPos);
				if (ss.bad()) {
					std::cerr << "Cannot parse fixed vertex argument: " << argv[currentArgument] << std::endl;
					break;
				}
				ss >> fixedVertexPos[i];
				fixedVertexArg = fixedVertexArg.substr(commaPos+1);
			}
			// Only store the first two fixed vertices. The whole markups fiducial list is passed to the CLI, but
			// we only need to use the first two for flattening. The rest are for the user for fitting.
			if (fixedVertices.size() < 2) {
				FixedVertexDefinition currentFixedVertex(fixedVertexPos[0], fixedVertexPos[1], fixedVertexPos[2]);
				fixedVertices.push_back(currentFixedVertex);
			}
		}
		// Fixed texture coordinates: --fixedTextureCoords or -c
		else if (!strcmp(argv[currentArgument], "--fixedTextureCoords") || !strcmp(argv[currentArgument], "-c")) {
			// Determine if the argument comes in individual coordinates or together and parse accordingly
			std::string fixedCoordArg(argv[currentArgument+1]);
			std::vector<std::string> fixedCoordsStrVector;
			if (fixedCoordArg.find(" ") == std::string::npos) {
				std::cout << "Fixed texture coordinates given separately in individual arguments" << std::endl;
				for (int i=0; i<fixedVertices.size(); ++i) {
					fixedCoordsStrVector.push_back(argv[currentArgument+1]);
					fixedCoordsStrVector.push_back(argv[currentArgument+2]);
					currentArgument += 2;
				}
			} else {
				std::cout << "Fixed texture coordinates given together in a single argument" << std::endl;
				size_t separatorPosition = fixedCoordArg.find(" ");
				while (separatorPosition != std::string::npos) {
					fixedCoordsStrVector.push_back(fixedCoordArg.substr(0, separatorPosition));
					fixedCoordArg = fixedCoordArg.substr(separatorPosition+1);
					separatorPosition = fixedCoordArg.find(" ");
				}
				if (!fixedCoordArg.empty()) {
					fixedCoordsStrVector.push_back(fixedCoordArg);
				}
				currentArgument++;
			}
			if (fixedCoordsStrVector.size() != fixedVertices.size() * 2) {
				std::cerr << "Failed to parse fixed texture coordinates correctly" << std::endl;
				return 1;
			}
			for (int i=0; i<fixedVertices.size(); ++i) {
				double fixedTexturePos[2] = {0.0, 0.0};
				std::stringstream ss;
				ss << fixedCoordsStrVector[2*i] << " " << fixedCoordsStrVector[2*i+1];
				ss >> fixedTexturePos[0] >> fixedTexturePos[1];
				std::cout << "Fixed texture coordinates #" << i << ": " << fixedTexturePos[0] << ", " << fixedTexturePos[1] << std::endl;
				fixedVertices[i].set_fixed_points(fixedTexturePos[0], fixedTexturePos[1]);
			}
		}
		// Input file
		else if (inFilePath.empty()) {
			inFilePath = std::string(argv[currentArgument]);
		}
		// Output file
		else {
			outFilePath = std::string(argv[currentArgument]);
		}
		currentArgument++;
	}

	std::cout << "--> Reading mesh..." << std::endl;
	Mesh mesh;
	mesh.read_obj(inFilePath.c_str());
	if (mesh.numVertices() == 0) {
		std::cerr << "Failed to read mesh from file " << inFilePath << std::endl;
		return 1;
	} else {
		std::cout << "Mesh read successfully. " << mesh.numVertices() << " vertices and " << mesh.numFaces() << " cells." << std::endl;
	}

	if (fixedVertices.size() > 0) {
		std::cout << "--> Applying fixed vertices..." << std::endl;
		mesh.apply_fixed_vertices(fixedVertices);
	}

	FormTrait traits(&mesh);

	std::cout << "--> Computing conformal map..." << std::endl;
	LSCM lscm(&mesh);
	lscm.project();

	std::cout << "--> Writing mesh..." << std::endl;
	mesh.write_obj(outFilePath.c_str());
	return 0;
}
