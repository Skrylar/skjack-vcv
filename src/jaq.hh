
#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#include <string>
#include "jack/jack.h"

// re-entering our zone of concern
#pragma GCC diagnostic pop
#pragma clang diagnostic pop

namespace jaq {
  struct client {
    jack_client_t* handle; // handle to our jack client

    static void* lib; // handle to the jack library
    static jack_nframes_t (*x_jack_get_buffer_size)(jack_client_t*);
    static jack_nframes_t (*x_jack_get_sample_rate)(jack_client_t*);
    static int (*x_jack_set_buffer_size_callback)(jack_client_t*, JackBufferSizeCallback, void*);
    static int (*x_jack_set_sample_rate_callback)(jack_client_t*, JackSampleRateCallback, void*);
    static int (*x_jack_set_process_callback)(jack_client_t*, JackProcessCallback, void*);
    static jack_client_t* (*x_jack_client_open)(const char*, unsigned long, jack_status_t*);
    static int (*x_jack_port_rename)(jack_client_t*, jack_port_t*, const char*);
    static int (*x_jack_port_unregister)(jack_client_t*, jack_port_t*);
    static jack_port_t* (*x_jack_port_register)(jack_client_t*, const char*, const char*, unsigned long, unsigned long);
    static void* (*x_jack_port_get_buffer)(jack_port_t*, jack_nframes_t);
    static int (*x_jack_activate)(jack_client_t*);

    static bool link(); // try to dynamically link to jack

    // these are publicly read-only; we do keep them updated via callbacks though
    jack_nframes_t buffersize_max;
    jack_nframes_t buffersize;
    jack_nframes_t samplerate;

    bool open(); // create the jack client
    bool close(); // destroy the jack client

    inline bool alive() const { return handle != 0; }

    inline void activate() {
      if (handle) x_jack_activate(handle);
    }

    inline void set_process_callback(JackProcessCallback cb, void* user) {
      if (handle) x_jack_set_process_callback(handle, cb, user);
    }

    client() : handle(0) {}

  private:
    client(const client&) {/*don't copy that floppy*/}

    static int on_jack_buffer_size(jack_nframes_t nframes, void* arg);
    static int on_jack_sample_rate(jack_nframes_t nframes, void* arg);
  };

  struct port {
    client* mom;
    jack_port_t* handle;

    port() : mom(0), handle(0) {}

    bool alive() const;

    bool register_audio(client& mom, const char* name, unsigned long flags);

    void unregister();

    // returns port memory as an array of samples; `samples` should match
    // whatever you recieved via the `nsamples` parameter
    inline jack_default_audio_sample_t* get_audio_buffer(jack_nframes_t nframes) {
      if (alive())
        return reinterpret_cast<jack_default_audio_sample_t*>(
          client::x_jack_port_get_buffer(handle, nframes));

      return NULL;
    }

    bool rename(const std::string& new_name);

  private:
    port(const port&) {/*don't ocopy that floppy*/}
  };
}
