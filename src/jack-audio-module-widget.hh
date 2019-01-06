#pragma once

#include "skjack.hh"

struct JackAudioModuleWidget;

struct JackPortLedTextField : LedDisplayTextField {
	int managed_port;
	JackAudioModuleWidget* master;

	JackPortLedTextField() : LedDisplayTextField() {}
};

struct JackAudioModuleWidget : ModuleWidget {
	TextField* port_names[8];

	JackAudioModuleWidget(JackAudioModule *module);

	virtual json_t *toJson() override;
	virtual void fromJson(json_t* json) override;
};
