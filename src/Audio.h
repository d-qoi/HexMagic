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
	Audio() {
		/*
		 *
		 * If something needs to go here, do it now
		 *
		 */
#ifdef __APPLE__
		this->backend = SoundIoBackendCoreAudio
#elif __WIN32
		this->backend = SoundIoBackendWasapi;
#elif __linux__
		this->backend = SoundIoBackendAlsa;
#endif
	}
	~Audio()
	{
		soundio_instream_destroy(this->instream);
		soundio_device_unref(this->input_device);
		soundio_destroy(this->soundio);
	}

	int init()
	{
		int err;
		this->soundio = soundio_create();
		if (!soundio) {
			printf("Unable to create soundio object\n");
			return 1;
		}
		return 0;
		if (backend == SoundIoBackendNone)
			err = soundio_connect(soundio);
		else
			err = soundio_connect_backend(soundio, backend);
		if (err) {
			printf("Unable to connect to soundio: %s\n", soundio_strerror(err));
			return err;
		}

		soundio_flush_events(soundio);
		this->default_input_device_num = soundio_default_input_device_index(soundio);
		if (default_input_device_num < 0) {
			printf("No default input device set\n");
			return -default_input_device_num;
		}


	}

private:
	enum SoundIoBackend backend = SoundIoBackendNone;
	int default_input_device_num;
	struct SoundIoDevice* input_device;
	struct SoundIo* soundio;
	struct SoundIoInStream* instream;

};

#endif /* SRC_AUDIO_H_ */
