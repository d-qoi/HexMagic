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
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(pickerProg);

		glUniformMatrix4fv(glGetUniformLocation(pickerProg, "P"), 1, GL_FALSE, &(*P)[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(pickerProg, "M"), 1, GL_FALSE, &M[0][0]);

		glBindBuffer(GL_ARRAY_BUFFER, zOffsetPickerBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, state.getModel().getZOffsetsBytes(), &state.getModel().getZOffsets()[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, zLengthPickerBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, state.getModel().getZLengthsBytes(), &state.getModel().getZLengths()[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Draw
		glBindVertexArray(pickVertexArray);
		glDrawElementsInstanced(GL_TRIANGLES, state.getModel().getElements().size(), GL_UNSIGNED_INT, 0, WIDTH*WIDTH);
		glBindVertexArray(0);
		glUseProgram(0);
		checkGLError("texture model");

		// Flush draw to ensure the pixel data is correct for intersection check
		glFlush();
		glFinish();

		if(!state.getMouseDown()) {
			checkIntersection(state);
		}

		// Render to display
		//clear the old frame
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//use shader
		glUseProgram(shaderProg);

        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "P"), 1, GL_FALSE, &(*P)[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "C"), 1, GL_FALSE, &C[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "M"), 1, GL_FALSE, &M[0][0]);
        glUniformMatrix3fv(glGetUniformLocation(shaderProg, "N"), 1, GL_FALSE, &N[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "L"), 1, GL_FALSE, &L[0][0]);
        glUniform4fv(glGetUniformLocation(shaderProg, "lightPos"), 1, &lightPos[0]);
        glUniform4fv(glGetUniformLocation(shaderProg, "camPos"), 1, &camPos[0]);

		float highlightX = -1;
		float highlightY = -1;
		if(state.getSelectedIndex() > 0) {
			highlightX = (state.getSelectedIndex() - 1) % WIDTH;
			highlightY = (state.getSelectedIndex() - 1) / WIDTH;
		}
		
		glUniform1f(glGetUniformLocation(shaderProg, "highlightX"), highlightX);
		glUniform1f(glGetUniformLocation(shaderProg, "highlightY"), highlightY);

		glBindBuffer(GL_ARRAY_BUFFER, zOffsetBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, state.getModel().getZOffsetsBytes(), &state.getModel().getZOffsets()[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, zLengthBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, state.getModel().getZLengthsBytes(), &state.getModel().getZLengths()[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//draw
		glBindVertexArray(vertexArray);
		glDrawElementsInstanced(GL_TRIANGLES, state.getModel().getElements().size(), GL_UNSIGNED_INT, 0, WIDTH*WIDTH);
		glBindVertexArray(0);
		glUseProgram(0);
        checkGLError("model");
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

	void checkIntersection(WorldState & state) {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		float colorData[4];

		if(state.getCursorX() < 0 || state.getCursorY() < 0) {
			// Cursor is not on screen
			if(state.getSelectedIndex() != 0) {
				state.setSelectedIndex(0);
			}

			return;
		}

		glReadPixels(state.getCursorX(), RESOLUTION - state.getCursorY(), 1, 1, GL_RGBA, GL_FLOAT, colorData);

		float y = colorData[3] * 255.0 - 1;
		float x = colorData[2] * 255.0 - 1;

		// Add 1 to index, as no selection is index 0
		int index = x + y * WIDTH + 1;

//		printf("%f %f\n", x, y);

		if(index != 0 && !state.getMouseDown()) {
			state.setSelectedIndex(index);
		}
	}


private:
	bool initialized;
	GLuint pickerProg;
	GLuint shaderProg;

	GLuint frameBuffer;
	GLuint renderBuffer;
	GLuint idRenderBuffer;

	GLuint zOffsetBuffer;
	GLuint zLengthBuffer;

	GLuint zOffsetPickerBuffer;
	GLuint zLengthPickerBuffer;

	GLuint vertexArray;

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
		pickerProg = ShaderManager::shaderFromFile(&texVertPath, &texFragPath, 1, 1);

		char const * vertPath = "resources/simple.vert";
		char const * fragPath = "resources/simple.frag";
		shaderProg = ShaderManager::shaderFromFile(&vertPath, &fragPath, 1, 1);
        
		checkGLError("shader");
	}

	void setupBuffers(Model & model)
	{
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		
		GLuint positionBuffer;
		GLuint normalBuffer;
		GLuint elementBuffer;

		GLint normalSlot;
        GLint positionSlot;

		GLuint xRectCoordBuffer;
		GLint xRectCoordSlot;
		GLuint yRectCoordBuffer;
		GLint yRectCoordSlot;

		GLint zOffsetSlot;
		GLint zLengthSlot;

		// Picker
		GLuint positionPickerBuffer;
		GLuint elementPickerBuffer;

		GLint positionPickerSlot;

		GLuint xRectCoordPickerBuffer;
		GLint xRectCoordPickerSlot;
		GLuint yRectCoordPickerBuffer;
		GLint yRectCoordPickerSlot;

		GLint zOffsetPickerSlot;
		GLint zLengthPickerSlot;
		
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
		glVertexAttribPointer(xRectCoordSlot, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(xRectCoordSlot, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("xrect coord setup");

		glGenBuffers(1, &yRectCoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, yRectCoordBuffer);
		if (loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getYRectCoordinatesBytes(), &model.getYRectCoordinates()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		yRectCoordSlot =    glGetAttribLocation(shaderProg, "yCoord");
		glEnableVertexAttribArray(yRectCoordSlot);
		glVertexAttribPointer(yRectCoordSlot, 1, GL_FLOAT, GL_FALSE, 0, 0);
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

		glGenBuffers(1, &zLengthBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, zLengthBuffer);
		if (loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getZLengthsBytes(), &model.getZLengths()[0], GL_DYNAMIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		zLengthSlot =    glGetAttribLocation(shaderProg, "zLength");
		glEnableVertexAttribArray(zLengthSlot);
		glVertexAttribPointer(zLengthSlot, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(zLengthSlot, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("zLength setup");

		// now the elements
		glGenBuffers(1, &elementBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
		if (loaded)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getElementBytes(), &model.getElements()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getElementBytes(), NULL, GL_STATIC_DRAW);
		//leave the element buffer active
        checkGLError("model setup");

		glBindVertexArray(0);

		// Setup picking buffers

		glGenVertexArrays(1, &pickVertexArray);
		glBindVertexArray(pickVertexArray);

		glGenBuffers(1, &positionPickerBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionPickerBuffer);
		if(loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), &model.getPosition()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		positionPickerSlot = glGetAttribLocation(pickerProg, "pos");
		glEnableVertexAttribArray(positionPickerSlot);
		glVertexAttribPointer(positionPickerSlot, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("p setup");

		glGenBuffers(1, &xRectCoordPickerBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, xRectCoordPickerBuffer);
		if (loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getXRectCoordinatesBytes(), &model.getXRectCoordinates()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		xRectCoordPickerSlot =    glGetAttribLocation(pickerProg, "xCoord");
		glEnableVertexAttribArray(xRectCoordPickerSlot);
		glVertexAttribPointer(xRectCoordPickerSlot, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(xRectCoordPickerSlot, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("xrect coord setup");

		glGenBuffers(1, &yRectCoordPickerBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, yRectCoordPickerBuffer);
		if (loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getYRectCoordinatesBytes(), &model.getYRectCoordinates()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		yRectCoordPickerSlot =    glGetAttribLocation(pickerProg, "yCoord");
		glEnableVertexAttribArray(yRectCoordPickerSlot);
		glVertexAttribPointer(yRectCoordPickerSlot, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(yRectCoordPickerSlot, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("yrect coord setup");

		glGenBuffers(1, &zOffsetPickerBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, zOffsetPickerBuffer);
		if (loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getZOffsetsBytes(), &model.getZOffsets()[0], GL_DYNAMIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		zOffsetPickerSlot =    glGetAttribLocation(pickerProg, "zOffset");
		glEnableVertexAttribArray(zOffsetPickerSlot);
		glVertexAttribPointer(zOffsetPickerSlot, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(zOffsetPickerSlot, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("zOffset setup");

		glGenBuffers(1, &zLengthPickerBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, zLengthPickerBuffer);
		if (loaded)
			glBufferData(GL_ARRAY_BUFFER, model.getZLengthsBytes(), &model.getZLengths()[0], GL_DYNAMIC_DRAW);
		else
			glBufferData(GL_ARRAY_BUFFER, model.getPositionBytes(), NULL, GL_STATIC_DRAW);
		zLengthPickerSlot =    glGetAttribLocation(pickerProg, "zLength");
		glEnableVertexAttribArray(zLengthPickerSlot);
		glVertexAttribPointer(zLengthPickerSlot, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(zLengthPickerSlot, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError("zLength setup");

		// now the elements
		glGenBuffers(1, &elementPickerBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementPickerBuffer);
		if (loaded)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getElementBytes(), &model.getElements()[0], GL_STATIC_DRAW);
		else
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getElementBytes(), NULL, GL_STATIC_DRAW);
		//leave the element buffer active
		checkGLError("model setup");

		glBindVertexArray(0);
		checkGLError("setup");
	}
};

#endif
