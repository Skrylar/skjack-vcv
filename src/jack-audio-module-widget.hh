#pragma once

#include "skjack.hh"

struct JackAudioModuleWidget : ModuleWidget {
	TextField* port_names[8];

	JackAudioModuleWidget(JackAudioModule *module);
   virtual ~JackAudioModuleWidget();

	/** \brief Hook to do something when a text widget reports a port has been named. */
	void on_port_renamed(int port, const std::string& name);

	virtual json_t *toJson() override;
	virtual void fromJson(json_t* json) override;
};
