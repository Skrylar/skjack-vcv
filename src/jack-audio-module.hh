#pragma once

#include "skjack.hh"
#include "dsp/resampler.hpp"
#include "dsp/ringbuffer.hpp"

#define AUDIO_OUTPUTS 8
#define AUDIO_INPUTS 8

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
	DoubleRingBuffer<Frame<AUDIO_INPUTS>, 8192> jack_input_buffer;
	DoubleRingBuffer<Frame<AUDIO_OUTPUTS>, 8192> jack_output_buffer;

	jack_port_t* jport;

	JackAudioModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		/* use the pointer to ourselves as a random unique port name;
		   TODO: persist this name to json when asked, and rename the port when loading the json */
		char port_name[128];
		snprintf(reinterpret_cast<char*>(&port_name), 128, "%p", reinterpret_cast<void*>(this));
		jport = jack_port_register(g_jack_client, reinterpret_cast<const char*>(&port_name), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

		inputSrc.setChannels(AUDIO_INPUT);
		outputSrc.setChannels(AUDIO_OUTPUT);

		g_audio_modules.push_back(this);
	}

	virtual ~JackAudioModule();

	void step() override;
};
