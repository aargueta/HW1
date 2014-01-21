#pragma once
#include <vector>
#include <map>

#ifndef _OBJIMPORTER_H_
#define _OBJIMPORTER_H_

class ObjImporter
{
public:
	ObjImporter(void);
	~ObjImporter(void);
	
	struct SceneObject{
		std::vector<float>*	vertices;
		std::vector<float>*	normals;
		std::vector<unsigned int>*	triIndices;
		std::vector<unsigned int>*	quadIndices;
		
		SceneObject(){
			vertices = new std::vector<float>();
			normals = new std::vector<float>();
			triIndices = new std::vector<unsigned int>();
			quadIndices = new std::vector<unsigned int>();
		}
	};

	bool ImportObj(const char* fileName, SceneObject& obj);

private:
	
	int vertCount;
	int normCount;
	int texCount;
	int triCount;
	int quadCount;

	struct vec3;
	struct vec2;	
	struct face;
	//struct NormFacePair;
	
	std::vector<float>	rawVertices;
	std::vector<float>	sceneVertices;
	std::vector<float>	sceneNormals;
	std::vector<unsigned int>	sceneTriIndices;
	std::vector<unsigned int>	sceneQuadIndices;
	std::vector<face*>	sceneFaces;
	std::vector<face> oldSceneFaces;

	std::map<int, std::map<int, std::vector<face*>>> vertNormPairs;
	

	void AssignNormals(ObjImporter::SceneObject& obj, std::vector<vec3> vertices, std::vector<vec3> normals);
	void PackObjectValues(ObjImporter::SceneObject& obj, std::vector<int> vertIndices, std::vector<int> normIndices, std::vector<vec3> vertices, std::vector<vec3> normals);
	
};

#endif

