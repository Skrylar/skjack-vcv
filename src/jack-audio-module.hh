#pragma once

#include "skjack.hh"
#include "dsp/resampler.hpp"
#include "dsp/ringbuffer.hpp"
#include "sr-latch.hh"

#define AUDIO_OUTPUTS 4
#define AUDIO_INPUTS 4
#define JACK_PORTS (AUDIO_OUTPUTS + AUDIO_INPUTS)

struct jack_audio_module_base: public Module {
   enum role_t {
      ROLE_DUPLEX,		// standard skjack module
      ROLE_OUTPUT		// all ports are outputs
   };

   role_t role;
   sr_latch output_latch;

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
   jaq::port jport[JACK_PORTS];

   void wipe_buffers();
   void globally_register();
   void globally_unregister();
   void assign_stupid_port_names();

   jack_audio_module_base(size_t params, size_t inputs,
			  size_t outputs, size_t lights);
   virtual ~jack_audio_module_base();
};

struct JackAudioModule: public jack_audio_module_base {
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
      NUM_LIGHTS
   };

   JackAudioModule();
   virtual ~JackAudioModule();

   void step() override;
};

struct jack_audio_out8_module: public jack_audio_module_base {
   enum ParamIds { NUM_PARAMS }; // none
   enum InputIds {
      ENUMS(AUDIO_INPUT, JACK_PORTS),
      NUM_INPUTS
   };
   enum OutputIds { NUM_OUTPUTS }; // none
   enum LightIds { NUM_LIGHTS };   // none

   jack_audio_out8_module();
   virtual ~jack_audio_out8_module();
   
   void step() override;
};
