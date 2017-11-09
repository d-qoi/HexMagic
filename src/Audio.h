/*
 * Audio.h
 *
 *  Created on: Nov 8, 2017
 *      Author: Alexander Hirshcfeld
 */

#ifndef SRC_AUDIO_H_
#define SRC_AUDIO_H_

#define SAMPLE_RATE (44100)

#include "Model.h"
#include "portaudio.h"

typedef struct {
	float left_phase;
	float right_phase;
} paData;

class Audio
{
public:
	static int streamCallback(const void* inputBuffer,
							void* outputBuffer,
							unsigned long frameCount,
							const PaStreamCallbackTimeInfo* timeInfo,
							PaStreamCallbackFlags statusFlags,
							void* userData)
	{
		return 0;
	}

	int init() {
		PaError err = Pa_Initialize();
		if (err != paNoError) {
			printf("Port Audio Initilization Error: %s\n", Pa_GetErrorText(err));
			return -1;
		}

		return 0;
	}

	int terminate() {
		PaError err = Pa_Terminate();
		if (err != paNoError) {
			printf("Port Audio Termination Error: %s\n", Pa_GetErrorText(err));
			return -1;
		}
		return 0;
	}

	void printDeviceList() {
		int numDev;
		numDev = Pa_GetDeviceCount();
		const PaDeviceInfo *devInfo;
		if (numDev < 0) {
			printf("Port Audio: Device Count Error: %s\n", Pa_GetErrorText((PaError)numDev));
			return;
		} else {
			printf("Found %d devices\n", numDev);
			for (int i = 0; i<numDev; i++) {
				devInfo = Pa_GetDeviceInfo(i);
				printf("Device %i :: %s\n", i, devInfo->name);
			}
		}
	}
private:

};

#endif /* SRC_AUDIO_H_ */
