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
	void init(std::string fileName)
    {
    	objLoader loader;
    	loader.load(fileName.c_str()); // This is going to assume that everything is between -0.5 and .5, Allowing for offsets, maybe? Later?
		if (loader.materialCount == 0)
			exit(EXIT_FAILURE);

		zOffsets = vector<GLfloat>();
		zLengths = vector<GLfloat>();
		for(int i = 0; i < WIDTH; i++) {
			for(int j = 0; j < WIDTH; j++) {
				zOffsets.push_back(i + j);
				zLengths.push_back(WIDTH + i + j);
			}
		}

		// Add the OBJ vertices to the model position vector
        positions = vector<GLfloat>(); // Verted
		elements = vector<GLuint>(); // Faces

		normals = vector<GLfloat>();

		xRectCoordinates = vector<GLfloat>();
		yRectCoordinates = vector<GLfloat>();

		positions = vector<GLfloat>();
		for(int i = 0; i < loader.vertexCount; i++) {
			double *array = loader.vertexList[i]->e;
			positions.push_back(array[0]);
			positions.push_back(array[1]);
			positions.push_back(array[2]);
		}

		// Add each face's vertex indices to the element list
		elements = vector<GLuint>();
		for(int i = 0; i < loader.faceCount; i++) {
			obj_face *face = loader.faceList[i];

			for(int j = 0; j < face->vertex_count; j++) {
				elements.push_back(face->vertex_index[j]);
			}
		}

		// Initialize a normal list. There is one normal per vertex
		vector<glm::vec3> normalVecs = vector<glm::vec3>(loader.vertexCount);

		// Loop over all faces and compute the normal for each vertex in the face.
		// Accumulate the computed normals into the correct vertex normal
		for(int i = 0; i < loader.faceCount; i++) {
			obj_face *face = loader.faceList[i];

			if(face->vertex_count != 3) {
				printf("ERROR: Face is not a triangle \n");
				return;
			}

			int vertex0Index = face->vertex_index[0];
			int vertex1Index = face->vertex_index[1];
			int vertex2Index = face->vertex_index[2];

			double *array0 = loader.vertexList[vertex0Index]->e;
			double *array1 = loader.vertexList[vertex1Index]->e;
			double *array2 = loader.vertexList[vertex2Index]->e;

			glm::vec3 vertex0 = glm::vec3(array0[0], array0[1], array0[2]);
			glm::vec3 vertex1 = glm::vec3(array1[0], array1[1], array1[2]);
			glm::vec3 vertex2 = glm::vec3(array2[0], array2[1], array2[2]);

			glm::vec3 edge0 = vertex1 - vertex0;
			glm::vec3 edge1 = vertex2 - vertex1;

			glm::vec3 normal = glm::cross(edge0, edge1);
			normalVecs[vertex0Index] += normal;
			normalVecs[vertex1Index] += normal;
			normalVecs[vertex2Index] += normal;
		}

		//DONE Loop over all the normals and make the unit length.
		normals = vector<GLfloat>();
		for(int i = 0; i < normalVecs.size(); i++) {
			glm::vec3 normal = glm::normalize(normalVecs[i]);

			normals.push_back(normal.x);
			normals.push_back(normal.y);
			normals.push_back(normal.z);
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

	GLfloat getZLength(int x, int y)
	{
		return getZLength(y * WIDTH + x);
	}

	GLfloat getZLength(int index)
	{
		return zLengths[index];
	}

	vector<GLfloat> const getPosition() const
	{ return positions; }
	
	vector<GLfloat> const getNormal() const
	{ return normals; }
	
	vector<GLuint> const getElements() const
	{ return elements; }

	vector<GLfloat> const getXRectCoordinates() const
	{ return xRectCoordinates; }

	vector<GLfloat> const getYRectCoordinates() const
	{ return yRectCoordinates; }

	vector<GLfloat> const getZOffsets() const
	{ return zOffsets; }

	vector<GLfloat> const getZLengths() const
	{ return zLengths; }

	vector<GLuint> const getModelIds() const
	{ return modelIds; }
	
	size_t getVertexCount() const
	{ return positions.size()/3; }
	
	size_t getPositionBytes() const
	{ return positions.size()*sizeof(GLfloat); }
	
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
	{ return zLengths.size()*sizeof(GLfloat); }

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
	vector<GLfloat> normals;
	vector<GLuint> elements;
	vector<GLuint> modelIds;
	vector<GLfloat> xRectCoordinates;
	vector<GLfloat> yRectCoordinates;
	vector<GLfloat> zOffsets;
	vector<GLfloat> zLengths;

	size_t objectCount;
    
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 dim;
    glm::vec3 center;
};

#endif
