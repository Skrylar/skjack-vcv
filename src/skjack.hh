#pragma once

// we don't control these, so don't complain to me about them
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#include <vector>
#include <condition_variable>
#include <mutex>

#include "rack.hpp"

// re-entering our zone of concern
#pragma GCC diagnostic pop
#pragma clang diagnostic pop

using namespace rack;

#include "jaq.hh"

struct JackAudioModule;
struct JackAudioModuleWidget;

struct jack_audio_module_base;
struct jack_audio_module_widget_base;

struct jack_audio_out8_module_widget;
struct jack_audio_out8_module;

extern std::condition_variable g_jack_cv;

// We'll be using this from here on out.
extern jaq::client g_jack_client;

extern std::mutex g_audio_modules_mutex;
extern std::vector<jack_audio_module_base*> g_audio_modules;
extern std::atomic<unsigned int> g_audio_blocked;

// Forward-declare the Plugin, defined in skjack.cc
extern Plugin *plugin;

// Forward-declare each Model, defined in each module source file
extern Model* jack_audio_model;
extern Model* jack_audio_out8_model;
