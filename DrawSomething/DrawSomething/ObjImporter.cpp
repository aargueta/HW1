#include "stdafx.h"
#include "ObjImporter.h"
#include <vector>
#include <map>

using namespace std;


ObjImporter::ObjImporter(void)
{
}

ObjImporter::~ObjImporter(void)
{
}

struct ObjImporter::vec3 {
	float x, y, z;

	void toArray(float xyz[3]){
		xyz[0] = x;
		xyz[1] = y;
		xyz[2] = z;
	}
};

struct ObjImporter::vec2 {
	float x, y;

	void toArray(float xy[2]){
		xy[0] = x;
		xy[1] = y;
	}
};

struct ObjImporter::face{
		bool isTri;
		int faceIndex;
		int vertIndex[4];
		int uvIndex[4];
		int normIndex[4];
		vec3 verts[4];
		vec2 uvs[4];
		vec3 norms[4];

		face(bool isTri, int faceIndex, unsigned int* vertIndex, unsigned int* uvIndex, unsigned int* normIndex){
			face::isTri = isTri;
			face::faceIndex = faceIndex;
			for(int i = 0; i < (isTri? 3 : 4); i++){
				face::vertIndex[i] = vertIndex[i];
				face::uvIndex[i] = uvIndex[i];
				face::normIndex[i] = normIndex[i];
			}
		}

		void SetVals(vec3* verts, vec2* uvs, vec3* norms){
			for(int i = 0; i < (isTri? 3 : 4); i++){
				face::verts[i] = verts[i];
				face::uvs[i] = uvs[i];
				face::norms[i] = norms[i];
			}
		}
	};

/*
struct ObjImporter::NormFacePair{
	int normIndex;
	vector<ObjImporter::face*> faces;

	NormFacePair(int normIndex, vector<ObjImporter::face*> faces){
		normIndex = normIndex;
		faces = faces;
	}
};*/

void ObjImporter::PackObjectValues(ObjImporter::SceneObject& obj, vector<int> vertIndices, vector<int> normIndices, vector<vec3> vertices, vector<vec3> normals){
	obj.vertices->clear();
	obj.normals->clear();
	for(int i = 0; i < vertIndices.size(); i++){
		float vertVals[3], normVals[3];
		vertices[vertIndices[i]].toArray(vertVals);
		normals[normIndices[i]].toArray(normVals);
		for(int j = 0; j < 3; j++){
			obj.vertices->push_back(vertVals[j]);
			obj.normals->push_back(normVals[j]);
		}
	}
	for(int i = 0; i < sceneFaces.size(); i++){
		face currFace = *sceneFaces[i];
		if(currFace.isTri){
			for(int j = 0; j < 3; j++){
				obj.triIndices->push_back(currFace.vertIndex[j] - 1);
			}
		}else{
			for(int j = 0; j < 3; j++){
				obj.quadIndices->push_back(currFace.vertIndex[j] - 1);
			}
		}
		
	}
}

void ObjImporter::AssignNormals(ObjImporter::SceneObject& obj, vector<vec3> vertices, vector<vec3> normals){
	vector<int> vertIndices, normIndices;
	// Fill normal and vertex indices vectors with known good data
	for(int i = 0; i < vertCount; i++){
		vertIndices.push_back(i);
		normIndices.push_back(vertNormPairs[i].begin()->first);
	}
	
	// Fix vertex/normal mismatch
	for(int i = 0; i < vertCount; i++){
		// Grab all normals for a vertex
		vector<pair<int, vector<face*>>> normFacePair(vertNormPairs[i].begin(), vertNormPairs[i].end());

		for(int j = 1; j < normFacePair.size(); j++){ // Skip the first reference
			// Duplicate vertices with multiple normals
			vertIndices.push_back(i);
			normIndices.push_back(normFacePair[j].first);

			// Update corresponding faces to point to duplicates
			vector<face*>* faces = &normFacePair[j].second;
			for(int k = 0; k < faces->size(); k++){
				face* face = (*faces)[k];
				for(int l = 0; l < (face->isTri? 3 : 4); l++){
					if(face->vertIndex[l] -1 == i && face->normIndex[l] - 1 == normFacePair[j].first){
						// Face Indexes are in OBJ format, therefore start at 1, NOT 0
						face->vertIndex[l] = vertIndices.size();
						face->normIndex[l] = normIndices.size();
					}
				}
			}
		}
	}
	
	PackObjectValues(obj, vertIndices, normIndices, vertices, normals);
}

bool ObjImporter::ImportObj(const char* fileName, ObjImporter::SceneObject& obj)
{
	FILE* file = NULL;
	errno_t err = fopen_s(&file, fileName, "r");
	if(err){
		printf("Error opening file.\n");
		return false;
	}

	int ret = 0;
	vector<vec3> vertices;
	vector<vec2> uvs;
	vector<vec3> normals;
	//	vector<face> faces;

	vertCount = 0;
	normCount = 0;
	texCount = 0;
	triCount = 0;
	quadCount = 0;
	while(ret != EOF){
		char lineHeader[128];
		ret = fscanf_s(file, "%s", lineHeader, _countof(lineHeader));
		if(ret == EOF)
			break;

		if(strcmp(lineHeader, "v") == 0){
			// Vertex found
			vertCount++;
			vec3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			sceneVertices.push_back(vertex.x);
			sceneVertices.push_back(vertex.y);
			sceneVertices.push_back(vertex.z);
			vertices.push_back(vertex);
		}else if(strcmp(lineHeader, "vt") == 0){
			// Vertex texure point found
			texCount++;
			vec2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y );
			uvs.push_back(uv);
		}else if(strcmp( lineHeader, "vn" ) == 0 ){
			// Normal found
			normCount++;
			vec3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			sceneNormals.push_back(normal.x);
			sceneNormals.push_back(normal.y);
			sceneNormals.push_back(normal.z);
			normals.push_back(normal);
		}else if(strcmp( lineHeader, "f" ) == 0 ){
			// Face found
			unsigned int vertIndex[4], uvIndex[4], normIndex[4];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", 
				&vertIndex[0], &uvIndex[0], &normIndex[0], 
				&vertIndex[1], &uvIndex[1], &normIndex[1], 
				&vertIndex[2], &uvIndex[2], &normIndex[2], 
				&vertIndex[3], &uvIndex[3], &normIndex[3] );

			if(matches == 9){
				triCount++;
				/*sceneTriIndices.push_back(vertIndex[0] - 1);
				sceneTriIndices.push_back(vertIndex[1] - 1);
				sceneTriIndices.push_back(vertIndex[2] - 1);*/
			}else if(matches == 12){
				quadCount++;
				/*sceneQuadIndices.push_back(vertIndex[0] - 1);
				sceneQuadIndices.push_back(vertIndex[1] - 1);
				sceneQuadIndices.push_back(vertIndex[2] - 1);
				sceneQuadIndices.push_back(vertIndex[3] - 1);*/
			}else{
				printf("Unexpected number of vertices in face!\n");
				return false;
			}

			// Organize by vert->norm->face for normal correction
			face* tempFace = new face((matches == 9), triCount + quadCount - 1, vertIndex, uvIndex, normIndex);
			sceneFaces.push_back(tempFace);
			oldSceneFaces.push_back(face(*tempFace));
			for(int i = 0; i < matches/3; i++){
				if(vertNormPairs.find(vertIndex[i] - 1) != vertNormPairs.end()){
					// Already contains entry for vertex, just add norm/face pair to it
					map<int, vector<face*>>* normFacePairs = &(vertNormPairs[vertIndex[i] - 1]);
					if(normFacePairs->find(normIndex[i] - 1) != normFacePairs->end()){
						// Vertex already contains reference to norm, just add face
						(*normFacePairs)[normIndex[i] - 1].push_back(tempFace);
					}else{
						// Add norm/face map
						vector<face*> tempFaceVector;
						tempFaceVector.push_back(tempFace);
						normFacePairs->insert(pair<int, vector<face*>> (normIndex[i] - 1, tempFaceVector));
					}
				}else{
					// Add entry for vertex/norm/face
					map<int, vector<face*>> tempNormFaceMap;
					vector<face*> tempFaceVector;
					tempFaceVector.push_back(tempFace);
					tempNormFaceMap.insert(pair<int, vector<face*>>(normIndex[i] - 1, tempFaceVector));
					vertNormPairs.insert(pair<int, map<int, vector<face*>>>(vertIndex[i] - 1, tempNormFaceMap));
				}
			}
		}else{
			// Discard line
			char testChar = '\0';
			while(testChar != '\n'){
				fscanf_s(file, "%c", &testChar, 1);
			}
		}
	}
	fclose(file);
	printf("File loaded: %d vertices, %d tex coords, %d norms, %d tris, %d quads\n", vertCount, texCount, normCount, triCount, quadCount);
	/*obj.vertices = new vector<float>(sceneVertices);
	obj.normals = new vector<float>(sceneNormals);
	obj.triIndices = new vector<unsigned int>(sceneTriIndices);
	obj.quadIndices = new vector<unsigned int>(sceneQuadIndices);*/

	AssignNormals(obj, vertices, normals);
}

