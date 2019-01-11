project('SkJack', 'c', 'cpp', version: '0.6.0')

slug = 'SkJack'

jack = dependency('jack', required: true)

add_project_arguments(['-DSLUG=' + slug, '-DVERSION=' + meson.project_version(), '-DARCH_LIN=1'], language: 'cpp')

add_project_arguments(['-Wall', '-Wextra', '-std=c++11', '-pedantic', '-g'], language: 'c')
add_project_arguments(['-Wall', '-Wextra', '-std=c++11', '-pedantic', '-g'], language: 'cpp')

add_project_link_arguments(['-ldl'], language: 'cpp')

# TODO: swap this with a subproject reference once we subordinate rack to a workspace
rack_include = include_directories('/opt/rack/include')

shared_module('plugin', [
'src/jack-audio-module.cc',
'src/jack-audio-module-widget.cc',
'src/skjack.cc',
'src/jaq.cc'],
name_prefix: '',
install: true,
install_dir: '.',
include_directories: [rack_include],
dependencies: [jack])

install_subdir('res', install_dir: '.')
