#include "jack-audio-module.hh"
#include <algorithm>

void JackAudioModule::step() {
	if (!jport) return;
	if (!g_jack_client) return;

	// == PREPARE SAMPLE RATE STUFF ==
	int sampleRate = (int) engineGetSampleRate();
	inputSrc.setRates(g_jack_samplerate, sampleRate);
	outputSrc.setRates(sampleRate, g_jack_samplerate);
	inputSrc.setChannels(AUDIO_INPUT);
	outputSrc.setChannels(AUDIO_OUTPUT);

	// == FROM RACK TO JACK ==
	if (!rack_output_buffer.full()) {
		Frame<AUDIO_OUTPUTS> outputFrame;
		for (int i = 0; i < AUDIO_OUTPUTS; i++) {
			outputFrame.samples[i] = inputs[AUDIO_INPUT + i].value / 10.f;
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
}

struct JackAudioModuleWidget : ModuleWidget {
	JackAudioModuleWidget(JackAudioModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/JackAudio.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 55.530807)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 0));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 55.530807)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 1));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 55.530807)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 2));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(38.506519, 55.530807)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 3));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069209, 70.144905)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 4));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 70.144905)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 5));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 70.144905)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 6));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(38.506519, 70.144905)), Port::INPUT, module, JackAudioModule::AUDIO_INPUT + 7));

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.7069209, 92.143906)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 0));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 92.143906)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 1));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 92.143906)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 2));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.506519, 92.143906)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 3));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.7069209, 108.1443)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 4));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.307249, 108.1443)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 5));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(26.906193, 108.1443)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 6));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.506523, 108.1443)), Port::OUTPUT, module, JackAudioModule::AUDIO_OUTPUT + 7));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 54.577202)), module, JackAudioModule::INPUT_LIGHT + 0));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 54.577202)), module, JackAudioModule::INPUT_LIGHT + 1));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 69.158226)), module, JackAudioModule::INPUT_LIGHT + 2));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 69.158226)), module, JackAudioModule::INPUT_LIGHT + 3));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 91.147583)), module, JackAudioModule::OUTPUT_LIGHT + 0));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 91.147583)), module, JackAudioModule::OUTPUT_LIGHT + 1));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 107.17003)), module, JackAudioModule::OUTPUT_LIGHT + 2));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 107.17003)), module, JackAudioModule::OUTPUT_LIGHT + 3));
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
