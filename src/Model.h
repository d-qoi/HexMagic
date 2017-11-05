#ifndef __MODEL
#define __MODEL

#include <vector>
#include "glm/glm.hpp"
#include "objload/objLoader.hpp"
using namespace std; //makes using vectors easy

#define WIDTH 16

struct RectModel {
	int x;
	int y;
	float zOffset;
	int zLength;
	int highlighted;
	int padding;
	int padding2;
	int padding3;
};

class Model
{
public:
	vector<RectModel> rects;

	void init()
    {
    	objLoader loader;
    	loader.load("resources/cube.obj"); // This is going to assume that everything is between -0.5 and .5, Allowing for offsets, maybe? Later?
		if (loader.materialCount == 0)
			exit(EXIT_FAILURE);

		rects = vector<RectModel>();

		for(int i = 0; i < WIDTH; i++) {
			for(int j = 0; j < WIDTH; j++) {
				RectModel rectModel = RectModel();
				rectModel.x = j;
				rectModel.y = i;
				rectModel.zOffset = 0;
				rectModel.zLength = 1;
				rectModel.highlighted = 0;
				rectModel.padding = 0;
				rectModel.padding2 = 0;
				rectModel.padding3 = 0;
				rects.push_back(rectModel);
			}
		}

		//DONE Add the OBJ vertices to the model position vector
        positions = vector<GLfloat>(); // Verted
		elements = vector<GLuint>(); // Faces
		colors = vector<GLfloat>(); // Colors

		normals = vector<GLfloat>();

		rectCoordinates = vector<GLuint>();

		// 8 vertices in each rect
		vector<glm::vec3> normalVecs = vector<glm::vec3>(rects.size() * 8);

		for(int i = 0; i < rects.size(); i++) {

			rects[i].zOffset = (float)(rects[i].x + rects[i].y);
			int xOffset = rects[i].x*2;
			int zOffset = rects[i].y*2;
			int yOffset = 0;//rects[i].zOffset;

			int positionOffset = positions.size();
			for (int j = 0; j < loader.vertexCount; j++) {
				positions.push_back(GLfloat(loader.vertexList[j]->e[0] + xOffset));
				positions.push_back(GLfloat(loader.vertexList[j]->e[1] + yOffset)); //
				positions.push_back(GLfloat(loader.vertexList[j]->e[2] + zOffset));
				rectCoordinates.push_back(rects[i].x);
				rectCoordinates.push_back(rects[i].y);
				modelIds.push_back((i+1) / 255);
				modelIds.push_back((i+1) % 255);
			}

			int elementOffset = elements.size();
			for (int j = 0; j < loader.faceCount; j++) {
				elements.push_back(GLuint((loader.faceList[j]->vertex_index[0]) + positionOffset/3));
				elements.push_back(GLuint((loader.faceList[j]->vertex_index[1]) + positionOffset/3));
				elements.push_back(GLuint((loader.faceList[j]->vertex_index[2]) + positionOffset/3));

//				colors.push_back(loader.materialList[loader.faceList[j]->material_index]->diff[0]);
//				colors.push_back(loader.materialList[loader.faceList[j]->material_index]->diff[1]);
//				colors.push_back(loader.materialList[loader.faceList[j]->material_index]->diff[2]);
				colors.push_back(i/64.0);
				colors.push_back(i/64.0);
				colors.push_back(i/64.0);
			}
			vector<glm::vec3> normalList;
			for (int j = 0; j < loader.vertexCount; j++) {
				normalList.push_back(glm::vec3(0));
			}

			for(int j = 0; j<loader.faceCount; j++) {
				glm::vec3 v0 = glm::vec3(
						loader.vertexList[loader.faceList[j]->vertex_index[0]]->e[0],
						loader.vertexList[loader.faceList[j]->vertex_index[0]]->e[1],
						loader.vertexList[loader.faceList[j]->vertex_index[0]]->e[2]
				);
				glm::vec3 v1 = glm::vec3(
						loader.vertexList[loader.faceList[j]->vertex_index[1]]->e[0],
						loader.vertexList[loader.faceList[j]->vertex_index[1]]->e[1],
						loader.vertexList[loader.faceList[j]->vertex_index[1]]->e[2]
				);
				glm::vec3 v2 = glm::vec3(
						loader.vertexList[loader.faceList[j]->vertex_index[2]]->e[0],
						loader.vertexList[loader.faceList[j]->vertex_index[2]]->e[1],
						loader.vertexList[loader.faceList[j]->vertex_index[2]]->e[2]
				);

				normalList[loader.faceList[j]->vertex_index[1]] += glm::cross(v0-v1, v1-v2);
				normalList[loader.faceList[j]->vertex_index[2]] += glm::cross(v1-v2, v2-v0);
				normalList[loader.faceList[j]->vertex_index[0]] += glm::cross(v2-v0, v0-v1);
			}
			for (int i = 0; i < normalList.size(); i++) {
				glm::vec3 norm = glm::normalize(normalList[i]);
				normals.push_back(norm.x);
				normals.push_back(norm.y);
				normals.push_back(norm.z);
			}
		}

        min = computeMinBound();
        max = computeMaxBound();
        center = computeCentroid();
        dim = computeDimension();
	}

	void setHighlight(int index)
	{
		RectModel model = rects[index];
		model.highlighted = 1;
		rects[index] = model;
	}

	void clearHighlight(int index) {
		RectModel model = rects[index];
		model.highlighted = 0;
		rects[index] = model;
	}

	vector<GLfloat> const getPosition() const
	{ return positions; }
	
	vector<GLfloat> const getColor() const
	{ return colors; }
	
	vector<GLfloat> const getNormal() const
	{ return normals; }
	
	vector<GLuint> const getElements() const
	{ return elements; }

	vector<GLuint> const getRectCoordinates() const
	{ return rectCoordinates; }

	vector<RectModel> getRects()
	{ return rects; }

	vector<GLuint> const getModelIds() const
	{ return modelIds; }
	
	size_t getVertexCount() const
	{ return positions.size()/3; }
	
	size_t getPositionBytes() const
	{ return positions.size()*sizeof(GLfloat); }
	
	size_t getColorBytes() const
	{ return colors.size()*sizeof(GLfloat); }
	
	size_t getNormalBytes() const
	{ return normals.size()*sizeof(GLfloat); }
	
	size_t getElementBytes() const
	{ return elements.size()*sizeof(GLuint); }

	size_t getRectCoordinatesBytes() const
	{ return rectCoordinates.size()*sizeof(GLuint); }

	size_t getModelIdBytes() const
	{ return modelIds.size()*sizeof(GLuint); }

    glm::vec3 getMinBound()
    { return min; }
    
    glm::vec3 getMaxBound()
    { return max; }
    
    glm::vec3 getCentroid()
    { return center; }
    
    glm::vec3 getDimension()
    { return dim; }

private:
	
	glm::vec3 computeMinBound()
	{
		glm::vec3 bound;
		
		for(int c=0; c<3; c++)
			bound[c] = std::numeric_limits<float>::max();
		
		for(int i=0; i<positions.size(); i+=3)
		{
			for(int c=0; c<3; c++)
			{
				if(positions[i+c] < bound[c])
					bound[c] = positions[i+c];
			}
		}
		
		return bound;
	}
	
	glm::vec3 computeMaxBound()
	{
		glm::vec3 bound;
		
		for(int c=0; c<3; c++)
			bound[c] = -std::numeric_limits<float>::max();
		
		for(int i=0; i<positions.size(); i+=3)
		{
			for(int c=0; c<3; c++)
			{
				if(positions[i+c] > bound[c])
					bound[c] = positions[i+c];
			}
		}
		
		return bound;
	}
	
	glm::vec3 computeCentroid()
	{
		glm::vec3 center = glm::vec3(0);
		float positionCount = 1.0f/(positions.size()/3.0f);
		
		for(int i=0; i<positions.size(); i+=3)
		{
			center[0] += positions[i] * positionCount;
			center[1] += positions[i+1] * positionCount;
			center[2] += positions[i+2] * positionCount;
		}
		
		return center;
	}
	
	glm::vec3 computeDimension()
	{
		glm::vec3 max = getMaxBound();
		glm::vec3 min = getMinBound();
		glm::vec3 dim = max - min;
		return dim;
	}
	
	vector<GLfloat> positions;
	vector<GLfloat> colors;
	vector<GLfloat> normals;
	vector<GLuint> elements;
	vector<GLuint> modelIds;
	vector<GLuint> rectCoordinates;

	size_t objectCount;
    
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 dim;
    glm::vec3 center;
};

#endif
