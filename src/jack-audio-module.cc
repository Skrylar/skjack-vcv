#include "jack-audio-module.hh"
#include <algorithm>

void JackAudioModule::step() {
	if (!jport) return;
	if (!g_jack_client) return;

	// == PREPARE SAMPLE RATE STUFF ==
	int sampleRate = (int) engineGetSampleRate();
	inputSrc.setRates(g_jack_samplerate, sampleRate);
	outputSrc.setRates(sampleRate, g_jack_samplerate);

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

struct JackAudioModuleWidget : ModuleWidget {
	TextField* port_names[8];

	JackAudioModuleWidget(JackAudioModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/JackAudio.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 10.530807)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 0));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 23.530807)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 1));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 36.530807)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 2));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 49.530807)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 3));

		port_names[0] = Widget::create<LedDisplayTextField>(mm2px(Vec(13.7069211, 8.530807)));
		port_names[0]->box.size = mm2px(Vec(35.0, 10.753));
		addChild(port_names[0]);

		port_names[1] = Widget::create<LedDisplayTextField>(mm2px(Vec(13.7069211, 21.530807)));
		port_names[1]->box.size = mm2px(Vec(35.0, 10.753));
		addChild(port_names[1]);

		port_names[2] = Widget::create<LedDisplayTextField>(mm2px(Vec(13.7069211, 34.530807)));
		port_names[2]->box.size = mm2px(Vec(35.0, 10.753));
		addChild(port_names[2]);

		port_names[3] = Widget::create<LedDisplayTextField>(mm2px(Vec(13.7069211, 47.530807)));
		port_names[3]->box.size = mm2px(Vec(35.0, 10.753));
		addChild(port_names[3]);

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 62.143906)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 0));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 75.143906)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 1));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 88.143906)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 2));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 101.143906)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 3));

		port_names[4] = Widget::create<LedDisplayTextField>(mm2px(Vec(13.7069211, 60.530807)));
		port_names[4]->box.size = mm2px(Vec(35.0, 10.753));
		addChild(port_names[4]);

		port_names[5] = Widget::create<LedDisplayTextField>(mm2px(Vec(13.7069211, 73.530807)));
		port_names[5]->box.size = mm2px(Vec(35.0, 10.753));
		addChild(port_names[5]);

		port_names[6] = Widget::create<LedDisplayTextField>(mm2px(Vec(13.7069211, 86.530807)));
		port_names[6]->box.size = mm2px(Vec(35.0, 10.753));
		addChild(port_names[6]);

		port_names[7] = Widget::create<LedDisplayTextField>(mm2px(Vec(13.7069211, 99.530807)));
		port_names[7]->box.size = mm2px(Vec(35.0, 10.753));
		addChild(port_names[7]);
	}
};

JackAudioModule::~JackAudioModule() {
	/* drop ourselves from active module list */
	auto x = std::find(g_audio_modules.begin(), g_audio_modules.end(), this);
	if (x != g_audio_modules.end())
		g_audio_modules.erase(x);

	/* and kill our port */
	if (jport) {
		jack_port_unregister(g_jack_client, jport);
	}
}

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *jack_audio_module = Model::create<JackAudioModule, JackAudioModuleWidget>("SkJack", "JackAudio", "JACK Audio", EXTERNAL_TAG);
