#pragma once

#include <vector>

#include <jack/jack.h>

#include "rack.hpp"

using namespace rack;

class JackAudioModule;

#include <condition_variable>
#include <mutex>

extern std::condition_variable g_jack_cv;

// We'll be using this from here on out.
extern jack_client_t* g_jack_client;
extern jack_nframes_t g_jack_buffersize;
extern jack_nframes_t g_jack_samplerate;

extern std::vector<JackAudioModule*> g_audio_modules;

// Forward-declare the Plugin, defined in skjack.cc
extern Plugin *plugin;

// Forward-declare each Model, defined in each module source file
extern Model *jack_audio_module;
