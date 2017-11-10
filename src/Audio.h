/*
 * Audio.h
 *
 *  Created on: Nov 8, 2017
 *      Author: Alexander Hirshcfeld
 */

#ifndef SRC_AUDIO_H_
#define SRC_AUDIO_H_

#define SAMPLE_COUNT 4096

#include "Model.h"

#include "glm/glm.hpp"
#include "SFML/Audio.hpp"
#include "simple_fft/fft_settings.h"
#include "simple_fft/fft.h"

class Audio : public sf::SoundRecorder
{
public:

	virtual bool onStart()
	{
		setProcessingInterval(processingInterval);
		this->initial_1D.resize(SAMPLE_COUNT+10);
		this->spectrum_1D.resize(SAMPLE_COUNT+10);
		return true;
	}

	virtual bool onProcessSamples(const sf::Int16* samples, std::size_t sampleCount)
	{
		if (!enabled)
			return true;
		std::fill(&this->initial_1D[0], &this->initial_1D[SAMPLE_COUNT], 0);
		bool err;
		const char* err_buff;
		double rollingAverageWeight;
		double real = 0;
		int offset = 0;
		int i, j;


		if (sampleCount <= SAMPLE_COUNT) {
			std::copy_n(samples, sampleCount, &initial_1D[0]);
		} else {
			std::copy_n(samples, SAMPLE_COUNT, &initial_1D[0]);
		}
		err = simple_fft::FFT(this->initial_1D, this->spectrum_1D, static_cast<size_t>(SAMPLE_COUNT), err_buff);
		if (!err) {
			printf("Error with FFT:: %s\n", err_buff);
		}

		switch (mode) {
		case 0: // Sound Energy
			rollingAverageWeight = 0.8;
			real = 0;
			for(i = 0; i<SAMPLE_COUNT/2; i++) {
				real += std::abs(spectrum_1D[i])/div;
			}
			real /= SAMPLE_COUNT/2;
			this->rollingAverage = real*rollingAverageWeight + (1-rollingAverageWeight)*rollingAverage;

			this->model->setZOffset(WIDTH/2, WIDTH/2, this->model->getZOffset(WIDTH/2, WIDTH/2) + rollingAverage*10);
			break;

		case 1: //FFT
			offset = fso;
			for(i = 0; i < WIDTH; i++) {
				real = 0;
				for (j = 0; j<(i)/4; j++) {
					real += std::abs(spectrum_1D[offset])/div;
					offset++;
				}
				real /= j;
				if (real > 0)
					this->model->setZOffset(WIDTH-i, WIDTH/2, this->model->getZOffset(WIDTH-i, WIDTH/2) + real/30);
			}
			break;
		default: // Random
			const int divcnt = 4;
			const int multipler = 5;
			rollingAverageWeight = 0.5;
			real = 0;
			for(i = 0; i<SAMPLE_COUNT/divcnt; i++) {
				real += std::abs(spectrum_1D[i])/div;
			}
			real /= SAMPLE_COUNT/divcnt;
			this->rollingAverage = real*rollingAverageWeight + (1-rollingAverageWeight)*rollingAverage;
			//printf("%f %f\n", rollingAverage, real);
			if ((int)std::log(real) > (int)std::log(rollingAverage)) { // std::log is ln, so this is checking if it's just about 3x larger
				int sel = std::rand()%(WIDTH*WIDTH);
				this->model->setZOffset(sel, this->model->getZOffset(sel) + real*multipler);
			}

			break;
		}

		return true;
	}

	virtual void onStop()
	{

	}
	void setModel(Model* model)
	{
		this->model = model;
	}
	void setModeSoundEnergy()
	{ this->mode = 0;}
	void setModeFFT()
	{ this->mode = 1;	}
	void setModeRandom()
	{ this->mode = 2; 	}
	void toggleMode()
	{	this->mode = (mode+1)%3;	}
	void enable()
	{ this->enabled = true;	}
	void disable()
	{ this->enabled = false;	}
	void toggle()
	{ this->enabled = !enabled;	}

private:

	Model *model;
	int mode = 0;
	bool enabled = false;
	sf::Time processingInterval = sf::milliseconds(100);

	RealArray1D initial_1D;
	ComplexArray1D spectrum_1D;

	const double div = 10000.0;
	// Make sure WIDTH*FFT_INCLUDE doesn't go over SAMPLE_COUNT

	double rollingAverage = 0;

	const int fso = 4; //fft start offset


};

#endif /* SRC_AUDIO_H_ */
