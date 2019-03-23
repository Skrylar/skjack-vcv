#pragma once

#include "skjack.hh"

struct jack_audio_module_widget_base: public ModuleWidget {
   TextField* port_names[8];

   jack_audio_module_widget_base(jack_audio_module_base* module);
   virtual ~jack_audio_module_widget_base();

   // hook to do something when a text widget reports a port has been
   // named.
   void on_port_renamed(int port, const std::string& name);

   // save and restore port names with the vcv file
   virtual json_t *toJson() override;
   virtual void fromJson(json_t* json) override;

   void assume_default_port_names();
};

struct JackAudioModuleWidget: public jack_audio_module_widget_base {
   JackAudioModuleWidget(JackAudioModule* module);
   virtual ~JackAudioModuleWidget();
};

struct jack_audio_out8_module_widget: public jack_audio_module_widget_base {
   jack_audio_out8_module_widget(jack_audio_out8_module* module);
   virtual ~jack_audio_out8_module_widget();
};

struct jack_audio_in8_module_widget: public jack_audio_module_widget_base {
   jack_audio_in8_module_widget(jack_audio_in8_module* module);
   virtual ~jack_audio_in8_module_widget();
};
