#ifndef __WORLDSTATE_H
#define __WORLDSTATE_H
#include <stdio.h>
#include "glm/ext.hpp"
#include "Model.h"
#include "Audio.h"
#include "SFML/Audio.hpp"
#include "SFML/Main.hpp"

#define NUM_TRACKED_FRAMES 10
#define SAMPLE_RATE 44100
#define PROCESS_TIME_MS 10

class WorldState
{
private:
	std::string const modelFiles[5] = {"resources/cube.obj", "resources/sphere.obj", "resources/hexagon.obj", "resources/teapot.obj", "resources/puzzle.obj"};
	int const modelFilesCount = 5;

	float frameTimes[NUM_TRACKED_FRAMES];
	float currentTime;
	bool running;
	Model model;
	int activeModel;
	int colorMode;
	bool perspective;

	glm::vec3 cameraPos;
	glm::vec3 cameraLook;
	glm::vec3 cameraUp;
	float camPitch;
	float camYaw;
	float camDistance;

	glm::vec4 lightPos;
	glm::vec3 lightIntensity;
	
	glm::mat4 lightRotate;
	glm::mat4 lightIncrement;
	glm::mat4 modelRotate;
	glm::mat4 modelIncrement;
	glm::mat4 modelTranslate;
	glm::mat4 cameraMatrix;

	glm::mat3 postKernel;
	int activeKernel;
	
	bool lightRotating;
	bool modelRotating;

	double cursorX;
	double cursorY;

	double windowWidth;
	double windowHeight;

	bool mouseDown;

	int selectedIndex;

	double velocity[WIDTH * WIDTH] = { 0 };
	double acceleration[WIDTH * WIDTH] = { 0 };

	Audio audio;

public:
	WorldState()
	{
		for(size_t i=0; i<NUM_TRACKED_FRAMES; i++)
			frameTimes[i] = 0.0f;
        
		running = true;

		activeModel = 0;

		reloadModel();

		glm::vec3 center = model.getCentroid();
		glm::vec3 min = model.getMinBound();
		glm::vec3 max = model.getMaxBound();
		glm::vec3 dim = model.getDimension();
        printf("model loaded: bounds");
		printf("[%.2f %.2f %.2f]..", min[0], min[1], min[2]);
		printf("[%.2f %.2f %.2f] ", max[0], max[1], max[2]);
		printf("= dim [%.2f %.2f %.2f]\n", dim[0], dim[1], dim[2]);
		camDistance = std::max(dim[0], dim[1]);
		camPitch = 0;
		camYaw = 0;
		// TODO: Alex fix
		cameraPos = glm::vec3(0, WIDTH/4, 4);
		cameraLook = glm::vec3(0, WIDTH/4.7f, 0);
		cameraUp = glm::vec3(0, 1, 0);

		lightPos = glm::vec4((max-center)*1.3f, 1);
        lightIntensity = glm::vec3(1,1,1);
        
        lightRotate = glm::mat4(1);
        lightIncrement = glm::rotate(glm::mat4(1), -0.05f, glm::vec3(0,1,0));
        
        modelRotate = glm::rotate(glm::mat4(1), (float)(3.0f*M_PI/4.0f), glm::vec3(0,1,0));
        modelIncrement = glm::rotate(glm::mat4(1), 0.02f, glm::vec3(0,1,0));
        modelTranslate = glm::translate(glm::mat4(1), -model.getCentroid());
		
		lightRotating = false;
		modelRotating = false;

		postKernel = glm::mat3(0, 0, 0, 0, 1, 0, 0, 0, 0);
		activeKernel = 0;

		// to get rid of other errors
		currentTime = 0;
		cursorX = 0;
		cursorY = 0;

		windowWidth = 0;
		windowHeight = 0;

		mouseDown = false;
		selectedIndex = 0;

		perspective = true;

		if (Audio::isAvailable()) {
			audio.setDevice(Audio::getDefaultDevice());
			audio.start(SAMPLE_RATE);
			audio.setModel(&this->model);
		}

		clearVelocities();
	}

	~WorldState()
	{
		audio.stop();
	}

	void reloadModel()
	{
		model = Model();
		model.init(modelFiles[activeModel]);

		// TODO: Add any additional cleanup necessary
	}
	
	void updateFrameTime(float timeAsSeconds)
	{
		for(size_t i=1; i<NUM_TRACKED_FRAMES; i++)
			frameTimes[i] = frameTimes[i-1];
		frameTimes[0] = timeAsSeconds;
	}
	
	void printFPS() const
	{
		float sum = 0.0f;
		float avg = 0.0f;
		float fps = 0.0f;
		
		for(size_t i=0; i<NUM_TRACKED_FRAMES; i++)
			sum += frameTimes[i];
		
		avg = sum / NUM_TRACKED_FRAMES;
		fps = 1.0f / avg;
		printf("fps %f\n", fps);
	}
	
	Model const & getModel() const
	{ return model; }
	
	void setRunning(bool r)
	{ running = r; }

	bool isRunning() const
	{ return running; }

	float getCurrentTime() const
	{ return this->currentTime; }

	void timeStep(float t)
	{
		float elapsed = t - this->currentTime;
		this->updateFrameTime(elapsed);
		
		//spin light
		if(lightRotating)
			lightRotate = lightIncrement * lightRotate;
        
        //spin model
		if(modelRotating)
			modelRotate = modelIncrement * modelRotate;
		
		this->currentTime = t;
	}
	
	Model & getModel()
	{ return model; }
	
	glm::mat4 getModelTranslate() const
	{ return modelTranslate; }
    
    glm::mat4 getModelRotate() const
    { return modelRotate; }
    
    glm::mat4 getLightRotate() const
    { return lightRotate; }
    
    glm::vec4 getLightPos() const
    { return this->lightPos; }
    
    glm::vec3 getLightIntensity() const
    { return this->lightIntensity; }
    
    glm::mat4 getCameraMatrix() const
    { return glm::lookAt(cameraPos, cameraLook, cameraUp); }

	void nextActiveModel()
	{
		this->activeModel++;
		if(this->activeModel > this->modelFilesCount - 1) {
			this->activeModel = 0;
		}

		printf("Model: %d\n", this->activeModel);

		reloadModel();
	}

	void nextActivePostProcessing()
	{
		this->activeKernel++;
		if(activeKernel > 3) {
			this->activeKernel = 0;
		}

		if(activeKernel == 0) {
			postKernel = glm::mat3(0, 0, 0, 0, 1, 0, 0, 0, 0);
		} else if(activeKernel == 1) {
			postKernel = glm::mat3(1, 1, 1, 1, -8, 1, 1, 1, 1);
		} else if(activeKernel == 2) {
			postKernel = glm::mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
		} else if (activeKernel == 3) {
			postKernel = glm::mat3(0, -1, 3, -2, 0, -1, 3, -2, 0);
		}

		printf("Kernel: %d\n", this->activeKernel);
	}

	glm::mat3 getPostKernel()
	{
		return postKernel;
	}

	void nextColorMode()
	{ this->colorMode = (colorMode + 1)%4; }

	int getColorMode()
	{ return colorMode;	}
    
    void togglePerspective()
    { this->perspective = !perspective; }

    bool getPerspective() const
    { return this->perspective; }

    glm::vec4 getCameraPos() const
    { return glm::vec4(this->cameraPos, 1); }
	
	void toggleModelRotate()
	{ modelRotating = !modelRotating; }
	
	void toggleLightRotate()
	{ lightRotating = !lightRotating; }

	void setCursorPos(sf::Vector2i pos, sf::Vector2u windowSize)
	{
		double x = (double)pos.x;
		double y = (double)pos.y;

		if(x < 0) {
			x = -1;
		} else if(x >= windowSize.x) {
			x = -1;
		}

		if(y <= 0) {
			y = -1;
		} else if(y > windowSize.y) {
			y = -1;
		}

		this->cursorX = x;
		this->cursorY = y;
	}

	double getCursorX()
	{ return cursorX; }

	double getCursorY()
	{ return cursorY; }

	void setWindowSize(sf::Vector2u windowSize)
	{
		windowWidth = windowSize.x;
		windowHeight = windowSize.y;
	}

	double getWindowWidth()
	{ return windowWidth; }

	double getWindowHeight()
	{ return windowHeight; }

	void moveCamera(double x, double y, double z)
	{
		printf("Move Camera\n");
		this->cameraPos += glm::vec3(x, y, z);
		this->cameraLook += glm::vec3(x, y, z);
	}

	void moveCameraForward(float amount)
	{
		glm::vec3 diff = amount * glm::normalize(this->cameraLook - this->cameraPos);
		this->cameraPos += diff;
		this->cameraLook += diff;
	}

	void moveCameraLook(float pitch, float yaw)
	{
		glm::mat4 base = glm::mat4(1.0f);
		glm::vec4 camBase = glm::vec4(0, WIDTH/4.7f, 0, 1) - glm::vec4(cameraPos, 1);
		this->camPitch += pitch;
		this->camYaw += yaw;

		glm::mat4 rotPitch = glm::rotate(base, this->camPitch, glm::vec3(1, 0, 0));
		glm::mat4 rotYaw = glm::rotate(base, this->camYaw, glm::vec3(0, 1, 0));

		glm::vec4 cameraLook = (rotYaw * rotPitch * camBase) + glm::vec4(cameraPos, 1);


		this->cameraLook = glm::vec3(cameraLook.x, cameraLook.y, cameraLook.z);
	}

	void setCameraLook(double pitch, double yaw)
	{

	}

	void setMouseDown(bool down)
	{
		this->mouseDown = down;
	}

	bool getMouseDown()
	{
		return mouseDown;
	}

	void setSelectedIndex(int index)
	{
		this->selectedIndex = index;
	}

	int getSelectedIndex()
	{
		return selectedIndex;
	}

	// Physics
	void clearVelocities()
	{
		for(int i = 0; i < WIDTH * WIDTH; i++) {
			velocity[i] = 0;
			acceleration[i] = 0;

			int x = i % WIDTH;
			int y = i / WIDTH;

			float equilibrium = x + y;
			model.setZOffset(i, equilibrium);
		}
	}

	void translateRect(int index, glm::ivec2 oldPos, glm::ivec2 newPos)
	{
		#define XY_SENSITIVITY 0.05f // might be helpful to scale translations in x and y

		if(index == 0) {
			return;
		}

		glm::vec2 diff = glm::vec2(newPos.x - oldPos.x, oldPos.y - newPos.y) * glm::vec2(XY_SENSITIVITY, XY_SENSITIVITY);
//		printf("Translating %f by %f\n", model.getZOffset(index - 1) , diff.y);
		model.setZOffset(index - 1, model.getZOffset(index - 1) + diff.y);
	}

	void toggleAudioProcessing()
	{ audio.toggle(); printf("Audio Toggle\n");	}

	void toggleAudioMode()
	{ audio.toggleMode(); printf("Mode Toggle\n");	}


	void tick(float elapsedTime)
	{
		// height -> accel constant for adj blocks
		const float K = 0.7f;
		// height -> accel constant 
		const float B = 0.1f;
		// velocity dampener
		const float P = 0.14f;
		// time fiddling
		const float t = min(elapsedTime * 20.0f, 0.5f);
		// acceleration dampener
		const float D = 0.955f;

		for(int i = 0; i < WIDTH * WIDTH; i++) {
			int x = i % WIDTH;
			int y = i / WIDTH;

			float equilibrium = x + y;
			float offset = model.getZOffset(i) - equilibrium;

			float sum = 0.0f;

			// update from local nearby
			if(x != 0) {
				// Previous x
				double prevOffset = model.getZOffset(i - 1) - (x - 1 + y);
				sum -= (offset - prevOffset) * K;
			}
			if(x != WIDTH-1) {
				// Next x
				double prevOffset = model.getZOffset(i + 1) - (x + 1 + y);
				sum -= (offset - prevOffset) * K;
			}

			if(y != 0) {
				// Previous y
				double prevOffset = model.getZOffset(i - WIDTH) - (y - 1 + x);
				sum -= (offset - prevOffset) * K;
			}

			if(y != WIDTH-1) {
				// Next y
				double prevOffset = model.getZOffset(i + WIDTH) - (y + 1 + x);
				sum -= (offset - prevOffset) * K;
			}

			acceleration[i] = acceleration[i] * D + sum - offset * B;// min(max(acceleration[i] * D + sum - offset * B, -50.0), 50.0);

			velocity[i] = (velocity[i] + acceleration[i] * t) * P;// min(max((velocity[i] + acceleration[i] * t) * P, -15.0), 15.0);
		}

		for(int i = 0; i < WIDTH * WIDTH; i++) {
			int x = i % WIDTH;
			int y = i / WIDTH;

			if(mouseDown && i == selectedIndex - 1) {
				// Do not mess with position of object if currently held by mouse
				continue;
			}
			float change = velocity[i] * t + 0.5 * acceleration[i] * t * t;
			if (change > -0.01f && change < 0.01f) {
				model.setZOffset(i, model.getZOffset(i));
			} else {
				model.setZOffset(i, model.getZOffset(i) + change);
				// min(max(model.getZOffset(i) + velocity[i] * t + 0.5 * acceleration[i] * t * t, -100.0), 100.0));
			}
		}
	}
};

#endif
