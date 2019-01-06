#pragma once

#include "skjack.hh"
#include "dsp/resampler.hpp"
#include "dsp/ringbuffer.hpp"

#define AUDIO_OUTPUTS 4
#define AUDIO_INPUTS 4
#define JACK_PORTS (AUDIO_OUTPUTS + AUDIO_INPUTS)

struct JackAudioModule : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(AUDIO_INPUT, AUDIO_INPUTS),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(AUDIO_OUTPUT, AUDIO_OUTPUTS),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(INPUT_LIGHT, AUDIO_INPUTS / 2),
		ENUMS(OUTPUT_LIGHT, AUDIO_OUTPUTS / 2),
		NUM_LIGHTS
	};

	int lastSampleRate = 0;
	int lastNumOutputs = -1;
	int lastNumInputs = -1;

	SampleRateConverter<AUDIO_INPUTS> inputSrc;
	SampleRateConverter<AUDIO_OUTPUTS> outputSrc;

	// in rack's sample rate
	DoubleRingBuffer<Frame<AUDIO_INPUTS>, 16> rack_input_buffer;
	DoubleRingBuffer<Frame<AUDIO_OUTPUTS>, 16> rack_output_buffer;
	DoubleRingBuffer<Frame<AUDIO_INPUTS>, (1<<15)> jack_input_buffer;
	DoubleRingBuffer<Frame<AUDIO_OUTPUTS>, (1<<15)> jack_output_buffer;

	std::mutex jmutex;
	jack_port_t* jport[JACK_PORTS];

	JackAudioModule();

	virtual ~JackAudioModule();

	void step() override;
};
