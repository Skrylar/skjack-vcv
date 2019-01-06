#include "jack-audio-module.hh"

#include <algorithm>

void JackAudioModule::step() {
	if (!jport) return;
	if (!g_jack_client) return;

	// == PREPARE SAMPLE RATE STUFF ==
	int sampleRate = (int) engineGetSampleRate();
	inputSrc.setRates(g_jack_samplerate, sampleRate);
	outputSrc.setRates(sampleRate, g_jack_samplerate);

	// == FROM JACK TO RACK ==
	if (rack_input_buffer.empty() && !jack_input_buffer.empty()) {
		int inLen = jack_input_buffer.size();
		int outLen = rack_input_buffer.capacity();
		inputSrc.process(jack_input_buffer.startData(), &inLen, rack_input_buffer.endData(), &outLen);
		jack_input_buffer.startIncr(inLen);
		rack_input_buffer.endIncr(outLen);
	}

	if (!rack_input_buffer.empty()) {
		Frame<AUDIO_OUTPUTS> input_frame = rack_input_buffer.shift();
		for (int i = 0; i < AUDIO_INPUTS; i++) {
			outputs[AUDIO_OUTPUT+i].value = input_frame.samples[i] * 10.0f;
		}
	}

	// == FROM RACK TO JACK ==
	if (!rack_output_buffer.full()) {
		Frame<AUDIO_OUTPUTS> outputFrame;
		for (int i = 0; i < AUDIO_OUTPUTS; i++) {
			outputFrame.samples[i] = inputs[AUDIO_INPUT + i].value / 10.0f;
		}
		rack_output_buffer.push(outputFrame);
	}

	if (rack_output_buffer.full()) {
		int inLen = rack_output_buffer.size();
		int outLen = jack_output_buffer.capacity();
		outputSrc.process(rack_output_buffer.startData(), &inLen, jack_output_buffer.endData(), &outLen);
		rack_output_buffer.startIncr(inLen);
		jack_output_buffer.endIncr(outLen);
	}

	// TODO: consider capping this? although an overflow here doesn't cause crashes...
	if (jack_output_buffer.size() > (g_jack_buffersize * 8)) {
		std::unique_lock<std::mutex> lock(jmutex);
		g_jack_cv.wait(lock);
	}
}

JackAudioModule::JackAudioModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	/* use the pointer to ourselves as a random unique port name;
		 TODO: persist this name to json when asked, and rename the port when loading the json */
	char port_name[128];

	if (g_jack_client) {
		for (int i = 0; i < JACK_PORTS; i++) {
			snprintf(reinterpret_cast<char*>(&port_name), 128, "%p:%d", reinterpret_cast<void*>(this), i);
			jport[i] = jack_port_register(g_jack_client, reinterpret_cast<const char*>(&port_name), JACK_DEFAULT_AUDIO_TYPE, (i < AUDIO_OUTPUTS ? JackPortIsOutput : JackPortIsInput), 0);
		}
	}

	inputSrc.setChannels(AUDIO_INPUT);
	outputSrc.setChannels(AUDIO_OUTPUT);

	g_audio_modules.push_back(this);
}

JackAudioModule::~JackAudioModule() {
	/* drop ourselves from active module list */
	auto x = std::find(g_audio_modules.begin(), g_audio_modules.end(), this);
	if (x != g_audio_modules.end())
		g_audio_modules.erase(x);

	/* and kill our port */
	if (g_jack_client == NULL) return;
	for (int i = 0; i < JACK_PORTS; i++) {
		jack_port_unregister(g_jack_client, jport[i]);
	}
}
