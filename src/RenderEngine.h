#ifndef __RENDERENGINE
#define __RENDERENGINE

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "GLHelper.h"
#include "WorldState.h"

class RenderEngine
{
public:
	RenderEngine()
	{
		initialized = false;
		
		//camera
        this->Perspective = glm::perspective(1.0f, 1.0f, 0.1f, 100.0f);
        this->OrthoPerspective = glm::ortho((float)-WIDTH, (float)WIDTH, (float)0, (float)WIDTH, 0.1f, 1000.0f);
        this->P = &OrthoPerspective;
        this->C = glm::mat4(1);
		this->M = glm::mat4(1);
	}

	~RenderEngine()
	{
		if(initialized)
		{
			// Clean up the buffers
			//glDeleteBuffers(1, &positionBuffer);
			//glDeleteBuffers(1, &colorBuffer);
		}
	}

	void init(WorldState & state)
	{
		initialized = true;

		lastHighlightedIndex = 0;

		float ver = initLoader();
		if( ver < 1.0f ) {
			printf("OpenGL is not supported.\n");
			exit(1);
		}
		printf("OpenGL version %.1f is supported.\n", ver);
		
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
        
		glm::vec3 dim = state.getModel().getDimension();
		float maxDim = std::max(dim[0], std::max(dim[1], dim[2]));
		this->Perspective = glm::perspective(1.0f, 1.0f, maxDim*0.01f, maxDim*10.0f);
        C = state.getCameraMatrix();

		setupShader();
		setupBuffers(state.getModel());
	}

	void display(WorldState & state)
	{
		this->C = state.getCameraMatrix();
		glm::mat4 mT = state.getModelTranslate();
		glm::mat4 mR = state.getModelRotate();
		glm::mat4 M = C*mR*mT;
		glm::mat3 N = glm::inverseTranspose(glm::mat3(M));
		glm::vec4 lightPos = state.getLightPos();
		glm::vec4 camPos = state.getCameraPos();
		glm::mat4 L = state.getLightRotate();

		if  (state.getPerspective())
			this->P = &Perspective;
		else
			this->P = &OrthoPerspective;

		// Render to framebuffer for picking
//		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
//		glClearColor(0, 0, 0, 0);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//		glUseProgram(pickTextureProg);
//
//		glUniformMatrix4fv(glGetUniformLocation(pickTextureProg, "P"), 1, GL_FALSE, &(*P)[0][0]);
//		glUniformMatrix4fv(glGetUniformLocation(pickTextureProg, "C"), 1, GL_FALSE, &C[0][0]);
//		glUniformMatrix4fv(glGetUniformLocation(pickTextureProg, "mR"), 1, GL_FALSE, &mR[0][0]);
//		glUniformMatrix4fv(glGetUniformLocation(pickTextureProg, "mT"), 1, GL_FALSE, &mT[0][0]);
//		glUniformMatrix4fv(glGetUniformLocation(pickTextureProg, "M"), 1, GL_FALSE, &M[0][0]);
//		glUniformMatrix3fv(glGetUniformLocation(pickTextureProg, "N"), 1, GL_FALSE, &N[0][0]);
//		glUniformMatrix4fv(glGetUniformLocation(pickTextureProg, "L"), 1, GL_FALSE, &L[0][0]);
//		glUniform4fv(glGetUniformLocation(pickTextureProg, "lightPos"), 1, &lightPos[0]);
//		glUniform4fv(glGetUniformLocation(pickTextureProg, "camPos"), 1, &camPos[0]);
//		glUniform1i(glGetUniformLocation(pickTextureProg, "shadingMode"), state.getShadingMode());
//
////		glBindBuffer(GL_UNIFORM_BUFFER, pickRectBuffer);
////		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(RectModel)*WIDTH*WIDTH, &state.getModel().getRects()[0]);
////		glBindBuffer(GL_UNIFORM_BUFFER, 0);
//
//		// Draw
//		glBindVertexArray(pickVertexArray);
//		glDrawElements(GL_TRIANGLES, state.getModel().getElements().size(), GL_UNSIGNED_SHORT, 0);
//		glBindVertexArray(0);
//		glUseProgram(0);
//		checkGLError("texture model");
//
//		glFlush();
//		glFinish();
//
//		if(!state.getMouseDown()) {
//			checkIntersection(state);
//		}

		// Render to display
		//clear the old frame
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //hacky light source size change
        GLfloat maxDis = state.getModel().getDimension()[2] * 3;
        GLfloat distScale = 1.0f / (glm::length(L*lightPos - camPos) / maxDis);
        glPointSize(glm::mix(1.0f, 10.0f, distScale));

        //printf("cam %f %f %f\n", camPos[0], camPos[1], camPos[2]);
        //printf("light %f %f %f\n", lightPos[0], lightPos[1], lightPos[2]);

		//use shader
		glUseProgram(shaderProg);

        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "P"), 1, GL_FALSE, &(*P)[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "C"), 1, GL_FALSE, &C[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "mR"), 1, GL_FALSE, &mR[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "mT"), 1, GL_FALSE, &mT[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "M"), 1, GL_FALSE, &M[0][0]);
        glUniformMatrix3fv(glGetUniformLocation(shaderProg, "N"), 1, GL_FALSE, &N[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "L"), 1, GL_FALSE, &L[0][0]);
        glUniform4fv(glGetUniformLocation(shaderProg, "lightPos"), 1, &lightPos[0]);
        glUniform4fv(glGetUniformLocation(shaderProg, "camPos"), 1, &camPos[0]);
        glUniform1i(glGetUniformLocation(shaderProg, "shadingMode"), state.getShadingMode());

		bool found = false;
		int highlightX = -1;
		int highlightY = -1;
		// TODO: Fix
//		for(int i = 0; i < WIDTH * WIDTH; i++) {
//			RectModel model = state.getModel().getRects()[i];
//
//			if(model.highlighted) {
//				highlightX = model.x;
//				highlightY = model.y;
//				found = true;
//
//				break;
//			}
//		}

		glUniform1i(glGetUniformLocation(shaderProg, "highlightX"), highlightX);
		glUniform1i(glGetUniformLocation(shaderProg, "highlightY"), highlightY);
		glUniform1i(glGetUniformLocation(shaderProg, "renderHighlight"), found);

//		glBindBuffer(GL_UNIFORM_BUFFER, rectBuffer);
//		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(RectModel)*WIDTH*WIDTH, &state.getModel().getRects()[0]);
//		glBindBuffer(GL_UNIFORM_BUFFER, 0);

//		int data;
//		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &data);
//
//		printf("%d\n", data);

		//draw
		glBindVertexArray(vertexArray);
//		glDrawElements(GL_TRIANGLES, state.getModel().getElements().size(), GL_UNSIGNED_SHORT, 0);
		glDrawElementsInstanced(GL_TRIANGLES, state.getModel().getElements().size(), GL_UNSIGNED_INT, 0, 40*40);
		glBindVertexArray(0);
		glUseProgram(0);
        checkGLError("model");

		glUseProgram(lightProg);

        glUniformMatrix4fv(glGetUniformLocation(lightProg, "P"), 1, GL_FALSE, &(*P)[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightProg, "C"), 1, GL_FALSE, &C[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightProg, "mR"), 1, GL_FALSE, &mR[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightProg, "mT"), 1, GL_FALSE, &mT[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(lightProg, "M"), 1, GL_FALSE, &M[0][0]);
		glUniformMatrix3fv(glGetUniformLocation(lightProg, "N"), 1, GL_FALSE, &N[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightProg, "L"), 1, GL_FALSE, &L[0][0]);
        glUniform4fv(glGetUniformLocation(lightProg, "lightPos"), 1, &lightPos[0]);
        glUniform4fv(glGetUniformLocation(lightProg, "camPos"), 1, &camPos[0]);
        glUniform1i(glGetUniformLocation(lightProg, "shadingMode"), state.getShadingMode());

        glBindVertexArray(lightArray);
        glDrawArrays(GL_POINTS, 1, GL_UNSIGNED_INT);
        glBindVertexArray(0);
        glUseProgram(0);
        checkGLError("light");
	}

	void buildRenderBuffers(size_t xSize, size_t ySize)
	{
		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		glGenRenderbuffers(1, &renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, xSize, ySize);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

		glGenRenderbuffers(1, &idRenderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, idRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, xSize, ySize);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, idRenderBuffer);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			fprintf(stderr, "Frame buffer setup failed : ");
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if(status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
				fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
			//if(status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
			//		fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS\n");
			if(status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
				fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
			if(status == GL_FRAMEBUFFER_UNSUPPORTED)
				fprintf(stderr, "GL_FRAMEBUFFER_UNSUPPORTED\n");
			exit(3);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		checkGLError("frame buffer");
	}

	int lastHighlightedIndex;

	void checkIntersection(WorldState & state) {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		unsigned char colorData[4];

		if(state.getCursorX() < 0 || state.getCursorY() < 0) {
			// Cursor is not on screen
			if(lastHighlightedIndex != 0) {
//				state.getModel().clearHighlight(lastHighlightedIndex - 1);
				lastHighlightedIndex = 0;
			}

			return;
		}

		glReadPixels(state.getCursorX(), RESOLUTION - state.getCursorY(), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, colorData);

		int y = colorData[3];
		int x = colorData[2];
		int index = x * 255 + y;

		if(index != 0 && !state.getMouseDown()) {
//			printf("Selected %d\n", index);
			state.setSelectedIndex(index);
//			state.getModel().setHighlight(index - 1);
			if(lastHighlightedIndex != 0 && lastHighlightedIndex != index) {
//				state.getModel().clearHighlight(lastHighlightedIndex - 1);
			}
			lastHighlightedIndex = index;
		}
	}


private:
	bool initialized;
	GLuint pickTextureProg;
	GLuint shaderProg;
    GLuint lightProg;

	GLuint frameBuffer;
	GLuint renderBuffer;
	GLuint idRenderBuffer;

	GLuint vertexArray;
    GLuint lightArray;

	GLuint pickVertexArray;
	
	glm::mat4 Perspective;
	glm::mat4 OrthoPerspective;
	glm::mat4 *P;
	glm::mat4 C;
	glm::mat4 M;
	
	float initLoader()
	{
		float ver = 0.0f;

		if (gl3wInit()) {
			fprintf(stderr, "failed to initialize OpenGL\n");
		}
		
		if (gl3wIsSupported(1, 1)) { ver = 1.1f; }
		if (gl3wIsSupported(1, 2)) { ver = 1.2f; }
		if (gl3wIsSupported(1, 3)) { ver = 1.3f; }
		if (gl3wIsSupported(1, 4)) { ver = 1.4f; }
		if (gl3wIsSupported(1, 5)) { ver = 1.5f; }
		
		if (gl3wIsSupported(2, 0)) { ver = 2.0f; }
		if (gl3wIsSupported(2, 1)) { ver = 2.1f; }
		
		if (gl3wIsSupported(3, 0)) { ver = 3.0f; }
		if (gl3wIsSupported(3, 1)) { ver = 3.1f; }
		if (gl3wIsSupported(3, 2)) { ver = 3.2f; }
		if (gl3wIsSupported(3, 3)) { ver = 3.3f; }
		
		if (gl3wIsSupported(4, 0)) { ver = 4.0f; }
		if (gl3wIsSupported(4, 1)) { ver = 4.1f; }
		if (gl3wIsSupported(4, 2)) { ver = 4.2f; }
		if (gl3wIsSupported(4, 3)) { ver = 4.3f; }
		if (gl3wIsSupported(4, 4)) { ver = 4.4f; }
		if (gl3wIsSupported(4, 5)) { ver = 4.5f; }
		
		return ver;
	}

	void setupShader()
	{
		char const * texVertPath = "resources/pickTexture.vert";
		char const * texFragPath = "resources/pickTexture.frag";
		pickTextureProg = ShaderManager::shaderFromFile(&texVertPath, &texFragPath, 1, 1);

		char const * vertPath = "resources/simple.vert";
		char const * fragPath = "resources/simple.frag";
		shaderProg = ShaderManager::shaderFromFile(&vertPath, &fragPath, 1, 1);
        
        char const * lightVPath = "resources/lightPos.vert";
		char const * lightFPath = "resources/lightPos.frag";
        lightProg = ShaderManager::shaderFromFile(&lightVPath, &lightFPath, 1, 1);

		checkGLError("shader");
	}

	void setupBuffers(Model & model)
	{
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		
		GLuint positionBuffer;
		GLuint colorBuffer;
		GLuint normalBuffer;
		GLuint elementBuffer;
		GLuint lightBuffer;

		GLuint positionTextureBuffer;
		GLuint modelIdBuffer;

        GLint colorSlot;
		GLint normalSlot;
        GLint positionSlot;

		GLint positionTextureSlot;
		GLint modelIdSlot;

		GLuint xRectCoordBuffer;
		GLint xRectCoordSlot;

		GLuint yRectCoordBuffer;
		GLint yRectCoordSlot;

		GLuint zOffsetBuffer;
		GLint zOffsetSlot;

		GLuint zLengthBuffer;
		GLint zLengthSlot;
		
		bool loaded = model.getPosition().size() > 0;

		//setup position buffer
		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		if(loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), &model.getPosition()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		positionSlot = glGetAttribLocation(shaderProg, "pos");
		glEnableVertexAttribArray(positionSlot);
		glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("p setup");

		// Do the same thing for the color data
		glGenBuffers(1, &colorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		if (loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getColorBytes(), &model.getColor()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		colorSlot =    glGetAttribLocation(shaderProg, "colorIn");
		glEnableVertexAttribArray(colorSlot);
		glVertexAttribPointer(colorSlot, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("color setup");

		// And normals
		glGenBuffers(1, &normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		if (loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getNormalBytes(), &model.getNormal()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		normalSlot =    glGetAttribLocation(shaderProg, "normalIn");
		glEnableVertexAttribArray(normalSlot);
		glVertexAttribPointer(normalSlot, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("n setup");

		glGenBuffers(1, &xRectCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, xRectCoordBuffer);
		if (loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getXRectCoordinatesBytes(), &model.getXRectCoordinates()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		xRectCoordSlot =    glGetAttribLocation(shaderProg, "xCoord");
		glEnableVertexAttribArray(xRectCoordSlot);
		glVertexAttribPointer(xRectCoordSlot, 1, GL_UNSIGNED_INT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glVertexAttribDivisor(xRectCoordSlot, 1);
		checkGLError("xrect coord setup");

		glGenBuffers(1, &yRectCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, yRectCoordBuffer);
		if (loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getYRectCoordinatesBytes(), &model.getYRectCoordinates()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		yRectCoordSlot =    glGetAttribLocation(shaderProg, "yCoord");
		glEnableVertexAttribArray(yRectCoordSlot);
		glVertexAttribPointer(yRectCoordSlot, 1, GL_UNSIGNED_INT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(yRectCoordSlot, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("yrect coord setup");

		glGenBuffers(1, &zOffsetBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, zOffsetBuffer);
		if (loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getZOffsetsBytes(), &model.getZOffsets()[0], GL_DYNAMIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		zOffsetSlot =    glGetAttribLocation(shaderProg, "zOffset");
		glEnableVertexAttribArray(zOffsetSlot);
		glVertexAttribPointer(zOffsetSlot, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(zOffsetSlot, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("zOffset setup");

//		glGenBuffers(1, &zLengthBuffer);
//		glBindBuffer(GL_ARRAY_BUFFER, zLengthBuffer);
//		if (loaded)
//			glBufferData(GL_ARRAY_BUFFER, model.getZLengthsBytes(), &model.getZLengths()[0], GL_DYNAMIC_DRAW);
//		else
//			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
//		zLengthSlot =    glGetAttribLocation(shaderProg, "zLength");
//		glEnableVertexAttribArray(zLengthSlot);
//		glVertexAttribPointer(zLengthSlot, 1, GL_UNSIGNED_INT, GL_FALSE, 0, 0);
//		glVertexAttribDivisor(zLengthSlot, 1);
//		glBindBuffer(GL_ARRAY_BUFFER, 0);
//		checkGLError("zLength setup");

		// now the elements
		glGenBuffers(1, &elementBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
		if (loaded)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getElementBytes(), &model.getElements()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getElementBytes(), NULL, GL_STATIC_DRAW);
		//leave the element buffer active
        checkGLError("model setup");

		//hacky way to draw the light
        glGenVertexArrays(1, &lightArray);
        glBindVertexArray(lightArray);
		glGenBuffers(1, &lightBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, lightBuffer);
		float lightPos[] = {1,1,1};
		glBufferData(GL_ARRAY_BUFFER, 3*sizeof(float), &lightPos, GL_STATIC_DRAW);
		glBindVertexArray(0);

		// Setup picking buffers

//		glGenVertexArrays(1, &pickVertexArray);
//		glBindVertexArray(pickVertexArray);
//
//		glGenBuffers(1, &positionTextureBuffer);
//		glBindBuffer(GL_ARRAY_BUFFER, positionTextureBuffer);
//		if(loaded)
//			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), &model.getPosition()[0], GL_STATIC_DRAW);
//		else
//			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
//		positionTextureSlot = glGetAttribLocation(pickTextureProg, "pos");
//		glEnableVertexAttribArray(positionTextureSlot);
//		glVertexAttribPointer(positionTextureSlot, 3, GL_FLOAT, GL_FALSE, 0, 0);
//		glBindBuffer(GL_ARRAY_BUFFER, 0);
//		checkGLError("p texture setup");
//
//		glGenBuffers(1, &modelIdBuffer);
//		glBindBuffer(GL_ARRAY_BUFFER, modelIdBuffer);
//		if(loaded)
//			glBufferData(GL_ARRAY_BUFFER, model.getModelIdBytes(), &model.getModelIds()[0], GL_STATIC_DRAW);
//		else
//			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
//		modelIdSlot = glGetAttribLocation(pickTextureProg, "modelId");
//		glEnableVertexAttribArray(modelIdSlot);
//		glVertexAttribPointer(modelIdSlot, 2, GL_UNSIGNED_INT, GL_FALSE, 0, 0);
//		glBindBuffer(GL_ARRAY_BUFFER, 0);
//		checkGLError("model id setup");
//
//		glGenBuffers(1, &elementBuffer);
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
//		if (loaded)
//			glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getElementBytes(), &model.getElements()[0], GL_STATIC_DRAW);
//		else
//			glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getElementBytes(), NULL, GL_STATIC_DRAW);
//		//leave the element buffer active
//		checkGLError("model setup");
//
//		glBindVertexArray(0);
		checkGLError("setup");
	}
};

#endif
