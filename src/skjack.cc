#include "skjack.hh"
#include "jack-audio-module.hh"

Plugin *plugin;
jaq::client g_jack_client;
std::condition_variable g_jack_cv;

// TODO: consider protecting this with a mutex
// this would be optimal for correctness, but in practice this gets modified
// very infrequently and nobody cares if we miss one or two audio frames right
// after creating or destroying a module...
std::mutex g_audio_modules_mutex;
std::vector<JackAudioModule*> g_audio_modules;
std::atomic<unsigned int> g_audio_blocked(0);

int on_jack_process(jack_nframes_t nframes, void *) {
	if (!g_jack_client.alive()) return 1;
	/* audio modules vector is blocked; we have to skip this cycle */
	if (!g_audio_modules_mutex.try_lock()) return 0;

	for (auto itr = g_audio_modules.begin();
      itr != g_audio_modules.end();
      itr++)
	{
		auto module = *itr;

		auto available = module->jack_output_buffer.size();
		if (available >= nframes) {
			jack_default_audio_sample_t* jack_buffer[JACK_PORTS];
			for (int i = 0; i < JACK_PORTS; i++) {
				jack_buffer[i] = module->jport[i].get_audio_buffer(nframes);
			}

			for (jack_nframes_t i = 0; i < nframes; i++) {
				Frame<AUDIO_OUTPUTS> output_frame = module->jack_output_buffer.shift();
				for (int j = 0; j < AUDIO_OUTPUTS; j++) {
					jack_buffer[j][i] = output_frame.samples[j];
				}

				Frame<AUDIO_INPUTS> input_frame;
				for (int j = 0; j < AUDIO_INPUTS; j++) {
					input_frame.samples[j] = jack_buffer[j+AUDIO_OUTPUTS][i];
				}
				module->jack_input_buffer.push(input_frame);
			}

			module->output_latch.reset();
		}
	}

	g_audio_blocked = 0;
	g_audio_modules_mutex.unlock();

	g_jack_cv.notify_all();
	return 0;
}

void init(Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	// Add all Models defined throughout the plugin
	p->addModel(jack_audio_module);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.

	/* prep a client object that will last the lifetime of the app;
	 * this is fine because individual modules will open ports belonging to us */

	if (jaq::client::link() && g_jack_client.open()) {
		g_jack_client.set_process_callback(&on_jack_process, NULL);
		g_jack_client.activate();
	}
}
