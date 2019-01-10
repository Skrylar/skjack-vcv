#pragma once

// we don't control these, so don't complain to me about them
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weverything"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#include <vector>
#include <condition_variable>
#include <mutex>

// because our users over in macOS have old jacks
#include <jack/weakjack.h>
#include <jack/jack.h>
#include "rack.hpp"

// re-entering our zone of concern
#pragma GCC diagnostic pop
#pragma clang diagnostic pop

using namespace rack;

struct JackAudioModule;

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
