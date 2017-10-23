#ifndef __MODEL
#define __MODEL

#include <vector>
#include "glm/glm.hpp"
#include "objload/objLoader.hpp"
using namespace std; //makes using vectors easy

class Model
{
public:
	struct RectModel {
		int x;
		int y;
		double zOffset;
		double zLength;
	};

	static const inline int faces[36] = {4, 0, 3,
										4, 3, 7,
										2, 6, 7,
										2, 7, 3,
										1, 5, 2,
										5, 6, 2,
										0, 4, 1,
										4, 5, 1,
										4, 7, 5,
										7, 6, 5,
										0, 1, 2,
										0, 2, 3};

    void init()
    {

		vector<RectModel> rects = vector<RectModel>();

		for(int i = 0; i < 8; i++) {
			for(int j = 0; j < 8; j++) {
				RectModel rectModel = RectModel();
				rectModel.x = i;
				rectModel.y = j;
				rectModel.zOffset = 0;
				rectModel.zLength = .8;
				rects.push_back(rectModel);
			}
		}

		objLoader loader;
//		loader.load("resources/cube.obj");
//		loader.load("resources/sphere.obj");
//		loader.load("resources/teapot.obj");

		//DONE Verify that the model has materials. If not, exit.
//        if(loader.materialCount < 1) {
//            printf("Object missing materials\n");
//            exit(1);
//        }

		//DONE Add the OBJ vertices to the model position vector
        positions = vector<GLfloat>();
//        for(int i = 0; i < loader.vertexCount; i++) {
//            double *array = loader.vertexList[i]->e;
//            positions.push_back(array[0]);
//            positions.push_back(array[1]);
//            positions.push_back(array[2]);
//        }

		elements = vector<GLuint>();
		colors = vector<GLfloat>();

		double width = .1;
		for(int i = 0; i < rects.size(); i++) {
			RectModel rectModel = rects[i];
			double x = rectModel.x * width - width/2;
			double z = rectModel.y * width - width/2;
			double y = rectModel.zOffset;

			// TODO: Refactor into loop
			positions.push_back(x + width);
			positions.push_back(y);
			positions.push_back(z);

			positions.push_back(x + width);
			positions.push_back(y);
			positions.push_back(z + width);

			positions.push_back(x);
			positions.push_back(y);
			positions.push_back(z + width);

			positions.push_back(x);
			positions.push_back(y);
			positions.push_back(z);

			positions.push_back(x + width);
			positions.push_back(y + rectModel.zLength);
			positions.push_back(z);

			positions.push_back(x + width);
			positions.push_back(y + rectModel.zLength);
			positions.push_back(z + width);

			positions.push_back(x);
			positions.push_back(y + rectModel.zLength);
			positions.push_back(z + width);

			positions.push_back(x);
			positions.push_back(y + rectModel.zLength);
			positions.push_back(z);

			for(int j = 0; j < 12; j++) {
				elements.push_back(i * 8 + faces[j * 3]);
				elements.push_back(i * 8 + faces[j * 3 + 1]);
				elements.push_back(i * 8 + faces[j * 3 + 2]);
			}

			for(int j = 0; j < 12; j++) {
				double color = i/64.0;

				colors.push_back(color);
				colors.push_back(color);
				colors.push_back(color);
			}
		}

		//DONE Add each face's vertex indices to the element list
//        elements = vector<GLuint>();
//		colors = vector<GLfloat>();
//        for(int i = 0; i < loader.faceCount; i++) {
//            obj_face *face = loader.faceList[i];
//
//            //DONE Get the color attribute for each vertex
//            //  Each face has a OBJ material index that you can use to get the material data.
//            //  Use the diffuse component as the color.
//            obj_material *material = loader.materialList[face->material_index];
//
//            for(int j = 0; j < face->vertex_count; j++) {
//                elements.push_back(face->vertex_index[j]);
//				colors.push_back(material->diff[0]);
//				colors.push_back(material->diff[1]);
//				colors.push_back(material->diff[2]);
//            }
//        }

		//DONE Initialize a normal list. There is one normal per vertex
		vector<glm::vec3> normalVecs = vector<glm::vec3>(rects.size() * 8);
        
        //DONE Loop over all faces and compute the normal for each vertex in the face.
		//  Accumulate the computed normals into the correct vertex normal
//		for(int i = 0; i < loader.faceCount; i++) {
//			obj_face *face = loader.faceList[i];
//
//			if(face->vertex_count != 3) {
//				printf("ERROR: Face is not a triangle \n");
//				return;
//			}
//
//			int vertex0Index = face->vertex_index[0];
//			int vertex1Index = face->vertex_index[1];
//			int vertex2Index = face->vertex_index[2];
//
//			double *array0 = loader.vertexList[vertex0Index]->e;
//			double *array1 = loader.vertexList[vertex1Index]->e;
//			double *array2 = loader.vertexList[vertex2Index]->e;
//
//			glm::vec3 vertex0 = glm::vec3(array0[0], array0[1], array0[2]);
//			glm::vec3 vertex1 = glm::vec3(array1[0], array1[1], array1[2]);
//			glm::vec3 vertex2 = glm::vec3(array2[0], array2[1], array2[2]);
//
//			glm::vec3 edge0 = vertex1 - vertex0;
//			glm::vec3 edge1 = vertex2 - vertex1;
//
//			glm::vec3 normal = glm::cross(edge0, edge1);
//			normalVecs[vertex0Index] += normal;
//			normalVecs[vertex1Index] += normal;
//			normalVecs[vertex2Index] += normal;
//		}

		//DONE Loop over all the normals and make the unit length.
		normals = vector<GLfloat>();
		for(int i = 0; i < normalVecs.size(); i++) {
//			glm::vec3 normal = glm::normalize(normalVecs[i]);
			glm::vec3 normal = glm::vec3(1, 0, 0);

			normals.push_back(normal.x);
			normals.push_back(normal.y);
			normals.push_back(normal.z);
		}

        min = computeMinBound();
        max = computeMaxBound();
        center = computeCentroid();
        dim = computeDimension();
	}

	vector<GLfloat> const getPosition() const
	{ return positions; }
	
	vector<GLfloat> const getColor() const
	{ return colors; }
	
	vector<GLfloat> const getNormal() const
	{ return normals; }
	
	vector<GLuint> const getElements() const
	{ return elements; }
	
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
	size_t objectCount;
    
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 dim;
    glm::vec3 center;
};

#endif
