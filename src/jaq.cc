#include "jaq.hh"

#include <dlfcn.h>

namespace jaq {

bool port::alive() const {
  return (mom && mom->alive() && handle);
}

bool port::register_audio(client& mom, const char* name, unsigned long flags) {
  if (!mom.alive()) return false;
  this->mom = &mom;
  handle = jack_port_register(
    mom.handle,
    name,
    JACK_DEFAULT_AUDIO_TYPE,
    flags,
    0);

  if (handle) return true;
  else {
    this->mom = 0;
    return false;
  }
}

bool port::rename(const std::string& new_name) {
  if (!alive()) return false;
  jack_port_rename(mom->handle, handle, new_name.c_str());
  // FIXME renames can actually fail, but detecting that is spotty
  return true;
}

void port::unregister() {
  if (alive()) jack_port_unregister(mom->handle, handle);
}

int client::on_jack_buffer_size(jack_nframes_t nframes, void* dptr) {
  auto self = reinterpret_cast<client*>(dptr);
	self->buffersize = nframes;
	return 0;
}

int client::on_jack_sample_rate(jack_nframes_t nframes, void* dptr) {
  auto self = reinterpret_cast<client*>(dptr);
	self->samplerate = nframes;
	return 0;
}

bool client::link() {
  return true;
}

bool client::open() {
  if (handle) return true;

  jack_status_t jstatus;
  handle = jack_client_open("VCV Rack", JackNoStartServer, &jstatus);
  if (!handle) return false;

  buffersize_max = jack_get_buffer_size(handle);
  buffersize = jack_get_buffer_size(handle);
  samplerate = jack_get_sample_rate(handle);

  jack_set_buffer_size_callback(handle, &on_jack_buffer_size, this);
  jack_set_sample_rate_callback(handle, &on_jack_sample_rate, this);

  return true;
}

bool client::close() {
  return false;
}

} /* namespace jaq */
