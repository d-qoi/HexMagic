#ifndef __MODEL
#define __MODEL

#include <vector>
#include "glm/glm.hpp"
#include "objload/objLoader.hpp"
using namespace std; //makes using vectors easy

#define WIDTH 40

class Model
{
public:
	void init()
    {
    	objLoader loader;
    	loader.load("resources/cube.obj"); // This is going to assume that everything is between -0.5 and .5, Allowing for offsets, maybe? Later?
		if (loader.materialCount == 0)
			exit(EXIT_FAILURE);

		zOffsets = vector<GLfloat>();
		zLengths = vector<GLuint>();
		for(int i = 0; i < WIDTH; i++) {
			for(int j = 0; j < WIDTH; j++) {
				zOffsets.push_back(i + j);
				zLengths.push_back(WIDTH + i + j);
			}
		}

		//DONE Add the OBJ vertices to the model position vector
        positions = vector<GLfloat>(); // Verted
		elements = vector<GLushort>(); // Faces
		colors = vector<GLfloat>(); // Colors

		normals = vector<GLfloat>();

		xRectCoordinates = vector<GLuint>();
		yRectCoordinates = vector<GLuint>();

		vector<glm::vec3> normalList;
		for (int j = 0; j < loader.vertexCount; j++) {
			positions.push_back(GLfloat(loader.vertexList[j]->e[0]));
			positions.push_back(GLfloat(loader.vertexList[j]->e[1])); //
			positions.push_back(GLfloat(loader.vertexList[j]->e[2]));

			normalList.push_back(glm::vec3(0));
		}

		int positionOffset = positions.size();

		for (int j = 0; j < loader.faceCount; j++) {
			elements.push_back(GLuint((loader.faceList[j]->vertex_index[0]) + positionOffset/3));
			elements.push_back(GLuint((loader.faceList[j]->vertex_index[1]) + positionOffset/3));
			elements.push_back(GLuint((loader.faceList[j]->vertex_index[2]) + positionOffset/3));

			//				colors.push_back(loader.materialList[loader.faceList[j]->material_index]->diff[0]);
			//				colors.push_back(loader.materialList[loader.faceList[j]->material_index]->diff[1]);
			//				colors.push_back(loader.materialList[loader.faceList[j]->material_index]->diff[2]);
			colors.push_back(1/64.0);
			colors.push_back(1/64.0);
			colors.push_back(1/64.0);

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

		for(int i = 0; i < WIDTH * WIDTH; i++) {
			int x = i % WIDTH;
			int y = i / WIDTH;

			xRectCoordinates.push_back(x);
			yRectCoordinates.push_back(y);
			modelIds.push_back((i+1) / 255);
			modelIds.push_back((i+1) % 255);
		}

        min = computeMinBound();
        max = computeMaxBound();
        center = computeCentroid();
        dim = computeDimension();
	}

	void setZOffset(int x, int y, float zOffset)
	{
		setZOffset(y * WIDTH + x, zOffset);
	}

	void setZOffset(int index, float zOffset)
	{
		zOffsets[index] = zOffset;
	}

	float getZOffset(int x, int y)
	{
		return getZOffset(y * WIDTH + x);
	}

	float getZOffset(int index)
	{
		return zOffsets[index];
	}

	void setZLength(int x, int y, int zLength)
	{
		setZLength(y * WIDTH + x, zLength);
	}

	void setZLength(int index, int zLength)
	{
		zLengths[index] = zLength;
	}

	int getZLength(int x, int y)
	{
		return getZLength(y * WIDTH + x);
	}

	int getZLength(int index)
	{
		return zLengths[index];
	}

	vector<GLfloat> const getPosition() const
	{ return positions; }
	
	vector<GLfloat> const getColor() const
	{ return colors; }
	
	vector<GLfloat> const getNormal() const
	{ return normals; }
	
	vector<GLushort> const getElements() const
	{ return elements; }

	vector<GLuint> const getXRectCoordinates() const
	{ return xRectCoordinates; }

	vector<GLuint> const getYRectCoordinates() const
	{ return yRectCoordinates; }

	vector<GLfloat> const getZOffsets() const
	{ return zOffsets; }

	vector<GLuint> const getZLengths() const
	{ return zLengths; }

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

	size_t getXRectCoordinatesBytes() const
	{ return xRectCoordinates.size()*sizeof(GLuint); }

	size_t getYRectCoordinatesBytes() const
	{ return yRectCoordinates.size()*sizeof(GLuint); }

	size_t getZOffsetsBytes() const
	{ return zOffsets.size()*sizeof(GLfloat); }

	size_t getZLengthsBytes() const
	{ return zLengths.size()*sizeof(GLuint); }

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
	vector<GLushort> elements;
	vector<GLuint> modelIds;
	vector<GLuint> xRectCoordinates;
	vector<GLuint> yRectCoordinates;
	vector<GLfloat> zOffsets;
	vector<GLuint> zLengths;

	size_t objectCount;
    
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 dim;
    glm::vec3 center;
};

#endif
