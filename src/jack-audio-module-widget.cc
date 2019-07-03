#include "jack-audio-module-widget.hh"
#include "jack-audio-module.hh"
#include "hashids.hh"
#include "components.hh"

namespace rack {
   extern std::shared_ptr<Font> gGuiFont;
}

struct JackPortLedTextField : public LedDisplayTextField {
   int managed_port;
   jack_audio_module_widget_base* master;

   JackPortLedTextField() : LedDisplayTextField() {
      font = APP->window->loadFont(asset::plugin(::plugin, "res/3270Medium.ttf"));
   }

   void draw(const DrawArgs &args) override {

    nvgScissor(args.vg, RECT_ARGS(args.clipBox));

   	// Background
   	nvgBeginPath(args.vg);
   	nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 5.0);
   	nvgFillColor(args.vg, nvgRGB(20, 39, 53));
   	nvgFill(args.vg);

   	// Text
   	if (font->handle >= 0) {
   		bndSetFont(font->handle);

   		NVGcolor highlightColor = nvgRGB(255, 255, 255);
   		highlightColor.a = 0.5;
   		int begin = std::min(cursor, selection);
   		int end = (this == APP->event->selectedWidget) ? std::max(cursor, selection) : -1;
   		bndIconLabelCaret(args.vg, textOffset.x, textOffset.y,
   			box.size.x - 2*textOffset.x, box.size.y - 2*textOffset.y,
   			-1, nvgRGB(255, 255, 255), 12, text.c_str(), highlightColor, begin, end);

   		bndSetFont(APP->window->uiFont->handle);
   	}

   	nvgResetScissor(args.vg);

   }

   void onChange(const event::Change &e) override {
      LedDisplayTextField::onChange(e);
      master->on_port_renamed(managed_port, text);
   }
};

jack_audio_module_widget_base::jack_audio_module_widget_base
(jack_audio_module_base* module):
   ModuleWidget(module)
{
}

#define def_port_label(id, x, y) {					\
   port_names[id] = createWidget<JackPortLedTextField>(mm2px(Vec(x, y))); \
   auto self = reinterpret_cast<JackPortLedTextField*>(port_names[id]);	\
   self->managed_port = id;						\
   self->master = this;							\
   self->box.size = mm2px(Vec(35.0, 10.753));				\
   addChild(self);							\
}

#define def_input(self, id, x, y) addInput				\
   (createInput<DavidLTPort>						\
    (mm2px(Vec(x, y)),							\
     module, self::AUDIO_INPUT + id));

#define def_output(self, id, x, y) addOutput				\
   (createOutput<DavidLTPort>						\
    (mm2px(Vec(x, y)),							\
     module, self::AUDIO_OUTPUT + id));

void jack_audio_module_widget_base::assume_default_port_names() {
   static const size_t buffer_size = 128;
   char port_name[buffer_size];
   hashidsxx::Hashids hash(g_hashid_salt);
   std::string id = hash.encode(reinterpret_cast<size_t>(module));

   for (int i = 0; i < JACK_PORTS; i++) {
      snprintf(reinterpret_cast<char*>(&port_name),
	       buffer_size,
	       "%s:%d", id.c_str(), i);
      // XXX using setText here would cause crashes because it would try to tell
      port_names[i]->text = std::string(port_name);
   }
}

JackAudioModuleWidget::JackAudioModuleWidget(JackAudioModule* module)
   : jack_audio_module_widget_base(module)
{
   setPanel(APP->window->loadSvg(asset::plugin(::plugin, "res/JackAudioB.svg")));

   addChild(createWidget<ScrewSilver>
	    (Vec(RACK_GRID_WIDTH, 0)));
   addChild(createWidget<ScrewSilver>
	    (Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
   addChild(createWidget<ScrewSilver>
	    (Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
   addChild(createWidget<ScrewSilver>
	    (Vec(box.size.x - 2 * RACK_GRID_WIDTH,
		 RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

   /*[[[cog
     for i in range(8):
       if i < 4:
         cog.outl('def_input(JackAudioModule, {}, 3.706, {});'.format(i, 10.530807 + (i * 13)))
       else:
         cog.outl('def_output(JackAudioModule, {}, 3.706, {});'.format(i % 4, 10.530807 + (i * 13)))
       cog.outl('def_port_label({}, 13.7069211, {});'.format(i, 8.530807 + (i * 13)))
     ]]] */
   def_input(JackAudioModule, 0, 3.706, 10.530807);
   def_port_label(0, 13.7069211, 8.530807);
   def_input(JackAudioModule, 1, 3.706, 23.530807);
   def_port_label(1, 13.7069211, 21.530807);
   def_input(JackAudioModule, 2, 3.706, 36.530806999999996);
   def_port_label(2, 13.7069211, 34.530806999999996);
   def_input(JackAudioModule, 3, 3.706, 49.530806999999996);
   def_port_label(3, 13.7069211, 47.530806999999996);
   def_output(JackAudioModule, 0, 3.706, 62.530806999999996);
   def_port_label(4, 13.7069211, 60.530806999999996);
   def_output(JackAudioModule, 1, 3.706, 75.530807);
   def_port_label(5, 13.7069211, 73.530807);
   def_output(JackAudioModule, 2, 3.706, 88.530807);
   def_port_label(6, 13.7069211, 86.530807);
   def_output(JackAudioModule, 3, 3.706, 101.530807);
   def_port_label(7, 13.7069211, 99.530807);
   //[[[end]]]

   assume_default_port_names();
}

jack_audio_out8_module_widget::jack_audio_out8_module_widget
(jack_audio_out8_module* module)
   : jack_audio_module_widget_base(module)
{
   setPanel(APP->window->loadSvg(asset::plugin(::plugin, "res/JackAudioB-8out.svg")));

   addChild(createWidget<ScrewSilver>
	    (Vec(RACK_GRID_WIDTH, 0)));
   addChild(createWidget<ScrewSilver>
	    (Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
   addChild(createWidget<ScrewSilver>
	    (Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
   addChild(createWidget<ScrewSilver>
	    (Vec(box.size.x - 2 * RACK_GRID_WIDTH,
		 RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

   /*[[[cog
     for i in range(8):
       cog.outl('def_input(jack_audio_out8_module, {}, 3.7069211, {});'.format(i, 10.530807 + (13 * i)))
       cog.outl('def_port_label({}, 13.7069211, {});'.format(i, 8.530807 + (i * 13)))
     ]]] */
   def_input(jack_audio_out8_module, 0, 3.7069211, 10.530807);
   def_port_label(0, 13.7069211, 8.530807);
   def_input(jack_audio_out8_module, 1, 3.7069211, 23.530807);
   def_port_label(1, 13.7069211, 21.530807);
   def_input(jack_audio_out8_module, 2, 3.7069211, 36.530806999999996);
   def_port_label(2, 13.7069211, 34.530806999999996);
   def_input(jack_audio_out8_module, 3, 3.7069211, 49.530806999999996);
   def_port_label(3, 13.7069211, 47.530806999999996);
   def_input(jack_audio_out8_module, 4, 3.7069211, 62.530806999999996);
   def_port_label(4, 13.7069211, 60.530806999999996);
   def_input(jack_audio_out8_module, 5, 3.7069211, 75.530807);
   def_port_label(5, 13.7069211, 73.530807);
   def_input(jack_audio_out8_module, 6, 3.7069211, 88.530807);
   def_port_label(6, 13.7069211, 86.530807);
   def_input(jack_audio_out8_module, 7, 3.7069211, 101.530807);
   def_port_label(7, 13.7069211, 99.530807);
   //[[[end]]]

   assume_default_port_names();
}

jack_audio_in8_module_widget::jack_audio_in8_module_widget
(jack_audio_in8_module* module)
   : jack_audio_module_widget_base(module)
{
   setPanel(APP->window->loadSvg(asset::plugin(::plugin, "res/JackAudioB-8in.svg")));

   addChild(createWidget<ScrewSilver>
	    (Vec(RACK_GRID_WIDTH, 0)));
   addChild(createWidget<ScrewSilver>
	    (Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
   addChild(createWidget<ScrewSilver>
	    (Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
   addChild(createWidget<ScrewSilver>
	    (Vec(box.size.x - 2 * RACK_GRID_WIDTH,
		 RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

   /*[[[cog
     for i in range(8):
       cog.outl('def_output(jack_audio_in8_module, {}, 3.7069211, {});'.format(i, 10.530807 + (13 * i)))
       cog.outl('def_port_label({}, 13.7069211, {});'.format(i, 8.530807 + (i * 13)))
       ]]] */
   def_output(jack_audio_in8_module, 0, 3.7069211, 10.530807);
   def_port_label(0, 13.7069211, 8.530807);
   def_output(jack_audio_in8_module, 1, 3.7069211, 23.530807);
   def_port_label(1, 13.7069211, 21.530807);
   def_output(jack_audio_in8_module, 2, 3.7069211, 36.530806999999996);
   def_port_label(2, 13.7069211, 34.530806999999996);
   def_output(jack_audio_in8_module, 3, 3.7069211, 49.530806999999996);
   def_port_label(3, 13.7069211, 47.530806999999996);
   def_output(jack_audio_in8_module, 4, 3.7069211, 62.530806999999996);
   def_port_label(4, 13.7069211, 60.530806999999996);
   def_output(jack_audio_in8_module, 5, 3.7069211, 75.530807);
   def_port_label(5, 13.7069211, 73.530807);
   def_output(jack_audio_in8_module, 6, 3.7069211, 88.530807);
   def_port_label(6, 13.7069211, 86.530807);
   def_output(jack_audio_in8_module, 7, 3.7069211, 101.530807);
   def_port_label(7, 13.7069211, 99.530807);
   //[[[end]]]

   assume_default_port_names();
}

#undef def_port_label
#undef def_input
#undef def_output

jack_audio_module_widget_base::~jack_audio_module_widget_base() {}
JackAudioModuleWidget::~JackAudioModuleWidget() {}
jack_audio_out8_module_widget::~jack_audio_out8_module_widget() {}
jack_audio_in8_module_widget::~jack_audio_in8_module_widget() {}

json_t* jack_audio_module_widget_base::toJson() {
   auto map = ModuleWidget::toJson();
   auto port_names = json_array();

   for (int i = 0; i < JACK_PORTS; i++) {
      auto str = json_string(this->port_names[i]->text.c_str());
      json_array_append_new(port_names, str);
   }

   json_object_set_new(map, "port_names", port_names);
   return map;
}

void jack_audio_module_widget_base::fromJson(json_t* json) {
   auto module = reinterpret_cast<JackAudioModule*>(this->module);
   auto port_names = json_object_get(json, "port_names");
   if (json_is_array(port_names)) {
      for (size_t i = 0; i < std::min(json_array_size(port_names), (size_t)8); i++) {
	 auto item = json_array_get(port_names, i);
	 if (json_is_string(item)) {
	    if (module->jport[i].rename(json_string_value(item))) {
	       this->port_names[i]->text = std::string(json_string_value(item));
	    } else {
	       static const size_t buffer_size = 128;
	       char port_name[buffer_size];
	       hashidsxx::Hashids hash(g_hashid_salt);
	       std::string id = hash.encode(reinterpret_cast<size_t>(module));

	       snprintf(reinterpret_cast<char*>(&port_name),
			buffer_size,
			"%s:%d", id.c_str(), (int)i);
	       this->port_names[i]->setText(std::string(port_name));
	    }
	 }
      }
   }
}

void jack_audio_module_widget_base::on_port_renamed(int port, const std::string& name) {
   if (port < 0 || port > JACK_PORTS) return;
   if (!g_jack_client.alive()) return;
   auto module = reinterpret_cast<JackAudioModule*>(this->module);
   if (!module) return;

   // XXX port names must be unique per client; using a non-unique name here
   // doesn't appear to "fail" but you do get a port with a blank name.
   if (!module->jport[port].rename(name)) {
     // warning was broken in 0.6->1.0
     DEBUG("Changing port name failed");
     //port_names[port]->setText(std::string(jack_port_short_name(module->jport[port])));
   }
}

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model* jack_audio_model =
   createModel<JackAudioModule, JackAudioModuleWidget>("JackAudio");

Model* jack_audio_out8_model =
   createModel<jack_audio_out8_module, jack_audio_out8_module_widget>("JackAudioOut8");

Model* jack_audio_in8_model =
   createModel<jack_audio_in8_module, jack_audio_in8_module_widget>("JackAudioIn8");
