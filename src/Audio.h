/*
 * Audio.h
 *
 *  Created on: Nov 8, 2017
 *      Author: Alexander Hirshcfeld
 */

#ifndef SRC_AUDIO_H_
#define SRC_AUDIO_H_

#define SAMPLE_RATE (44100)

#include "soundio/soundio.h"
#include "Model.h"

class Audio
{
public:
	int init()
	{
		enum SoundIoBackend backend = SoundIoBackendNone;
#ifdef __APPLE__
		backend =
#elif __WIN32

#elif __linux__

#endif
		return 0;
	}

private:

};

#endif /* SRC_AUDIO_H_ */
