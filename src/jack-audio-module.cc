#include "jack-audio-module.hh"
#include "hashids.hh"

#include <algorithm>

void JackAudioModule::step() {
   if (!g_jack_client.alive()) return;

   // == PREPARE SAMPLE RATE STUFF ==
   int sampleRate = (int) engineGetSampleRate();
   inputSrc.setRates(g_jack_client.samplerate, sampleRate);
   outputSrc.setRates(sampleRate, g_jack_client.samplerate);

   // == FROM JACK TO RACK ==
   if (rack_input_buffer.empty() && !jack_input_buffer.empty()) {
      int inLen = jack_input_buffer.size();
      int outLen = rack_input_buffer.capacity();
      inputSrc.process
	 (jack_input_buffer.startData(),
	  &inLen, rack_input_buffer.endData(), &outLen);
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
      outputSrc.process
	 (rack_output_buffer.startData(),
	  &inLen, jack_output_buffer.endData(), &outLen);
      rack_output_buffer.startIncr(inLen);
      jack_output_buffer.endIncr(outLen);
   }

   // TODO: consider capping this? although an overflow here doesn't cause crashes...
   if (jack_output_buffer.size() > (g_jack_client.buffersize * 8)) {
      // we're over half capacity, so set our output latch
      if (output_latch.try_set()) {
	 g_audio_blocked++;
      }

      // if everyone is output latched, stall Rack
      if (g_audio_blocked >= g_audio_modules.size()) {
	 std::unique_lock<std::mutex> lock(jmutex);
	 g_jack_cv.wait(lock);
      }
   }
}

void jack_audio_module_base::assign_stupid_port_names() {
   // TODO deduplicate with same code on the widget side
   /* use the pointer to ourselves as a random unique port name */
   char port_name[128];
   if (g_jack_client.alive()) {
      hashidsxx::Hashids hash("grilled cheese sandwiches");
      std::string id = hash.encode(reinterpret_cast<size_t>(this));

      for (int i = 0; i < JACK_PORTS; i++) {	  
	 snprintf
	    (reinterpret_cast<char*>(&port_name),
	     128,
	     "%s:%d",
	     id.c_str(),
	     i);
	    
	 jport[i].register_audio
	    (g_jack_client,
	     reinterpret_cast<const char*>(&port_name),
	     (i < AUDIO_OUTPUTS ? JackPortIsOutput : JackPortIsInput));
      }
   }
}

JackAudioModule::JackAudioModule()
   : jack_audio_module_base(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
{
   assign_stupid_port_names();

   inputSrc.setChannels(AUDIO_INPUTS);
   outputSrc.setChannels(AUDIO_OUTPUTS);

   globally_register();
}

jack_audio_module_base::jack_audio_module_base
(size_t params, size_t inputs, size_t outputs, size_t lights)
   : Module(params, inputs, outputs, lights),
     role(ROLE_DUPLEX),
     output_latch(), inputSrc(), outputSrc()
{
}

jack_audio_module_base::~jack_audio_module_base() {
   // unregister from client
   globally_unregister();

   // kill our port
   if (!g_jack_client.alive()) return;
   for (int i = 0; i < JACK_PORTS; i++) {
      jport[i].unregister();
   }
}

void jack_audio_module_base::wipe_buffers() {
   rack_input_buffer.clear();
   rack_output_buffer.clear();
   jack_input_buffer.clear();
   jack_output_buffer.clear();
}

void jack_audio_module_base::globally_register() {
   std::unique_lock<std::mutex> lock(g_audio_modules_mutex);

   g_audio_modules.push_back(this);

   /* ensure modules are not filling up their buffers out of sync */
   for (auto itr = g_audio_modules.begin();
	itr != g_audio_modules.end();
	itr++)
   {
      (*itr)->wipe_buffers();
   }
}

void jack_audio_module_base::globally_unregister() {
   std::unique_lock<std::mutex> lock(g_audio_modules_mutex);

   /* drop ourselves from active module list */
   auto x = std::find(g_audio_modules.begin(), g_audio_modules.end(), this);
   if (x != g_audio_modules.end())
      g_audio_modules.erase(x);
}

JackAudioModule::~JackAudioModule() {}

jack_audio_out8_module::jack_audio_out8_module()
   : jack_audio_module_base(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
{
   role = ROLE_OUTPUT;
   assign_stupid_port_names();

   inputSrc.setChannels(AUDIO_INPUTS);
   outputSrc.setChannels(AUDIO_OUTPUTS);

   globally_register();
}

jack_audio_out8_module::~jack_audio_out8_module() {}

void jack_audio_out8_module::step() {
   if (!g_jack_client.alive()) return;

   // == PREPARE SAMPLE RATE STUFF ==
   int sampleRate = (int) engineGetSampleRate();
   // not a bug; we're abusing both input and output pipes to be output pipes
   inputSrc.setRates(g_jack_client.samplerate, sampleRate);
   outputSrc.setRates(g_jack_client.samplerate, sampleRate);

   // == FROM RACK TO JACK ==
   if (!rack_output_buffer.full()) {
      Frame<AUDIO_OUTPUTS> outputFrame;
      for (int i = 0; i < 4; i++) {
	 outputFrame.samples[i] = inputs[AUDIO_INPUT + i].value / 10.0f;
      }
      rack_output_buffer.push(outputFrame);

      for (int i = 0; i < 4; i++) {
	 outputFrame.samples[i] = inputs[AUDIO_INPUT + i + 4].value / 10.0f;
      }
      rack_input_buffer.push(outputFrame);
   }

   if (rack_output_buffer.full()) {
      int inLen = rack_output_buffer.size();
      int outLen = jack_output_buffer.capacity();
      outputSrc.process
	 (rack_output_buffer.startData(),
	  &inLen, jack_output_buffer.endData(), &outLen);
      rack_output_buffer.startIncr(inLen);
      jack_output_buffer.endIncr(outLen);
   }

   if (rack_input_buffer.full()) {
      int inLen = rack_input_buffer.size();
      int outLen = jack_input_buffer.capacity();
      inputSrc.process
	 (rack_input_buffer.startData(),
	  &inLen, jack_input_buffer.endData(), &outLen);
      rack_input_buffer.startIncr(inLen);
      jack_input_buffer.endIncr(outLen);
   }

   // TODO: consider capping this?
   // although an overflow here doesn't cause crashes...
   if ((jack_output_buffer.size() > (g_jack_client.buffersize * 8)) ||
       ( jack_input_buffer.size() > (g_jack_client.buffersize * 8)))
   {
      // we're over half capacity, so set our output latch
      if (output_latch.try_set()) {
	 g_audio_blocked++;
      }

      // if everyone is output latched, stall Rack
      if (g_audio_blocked >= g_audio_modules.size()) {
	 std::unique_lock<std::mutex> lock(jmutex);
	 g_jack_cv.wait(lock);
      }
   }
}
