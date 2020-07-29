libgmpd_dependencies = [
  dependency('glib-2.0'),
  dependency('gobject-2.0'),
  dependency('gio-2.0'),
  dependency('gio-unix-2.0'),
]

libgmpd_include_dirs = include_directories('.')

libgmpd_version = '0.0.1'

libgmpd_sources = [
  'gmpd-audio-format.c',
  'gmpd-client.c',
  'gmpd-entity-priv.h',
  'gmpd-entity.c',
  'gmpd-idle.c',
  'gmpd-idle-response.c',
  'gmpd-idle-response.h',
  'gmpd-error.c',
  'gmpd-object.c',
  'gmpd-object.h',
  'gmpd-option-state.c',
  'gmpd-playback-state.c',
  'gmpd-protocol.c',
  'gmpd-protocol.h',
  'gmpd-response.c',
  'gmpd-response.h',
  'gmpd-song.c',
  'gmpd-stats.c',
  'gmpd-status.c',
  'gmpd-tag.c',
  'gmpd-version.c',
]

libgmpd_headers = [
  'gmpd.h',
  'gmpd-audio-format.h',
  'gmpd-client.h',
  'gmpd-entity.h',
  'gmpd-error.h',
  'gmpd-idle.h',
  'gmpd-option-state.h',
  'gmpd-playback-state.h',
  'gmpd-song.h',
  'gmpd-stats.h',
  'gmpd-status.h',
  'gmpd-tag.h',
  'gmpd-version.h',
]

shared_library('gmpd', libgmpd_sources,
  dependencies: libgmpd_dependencies,
  include_directories: libgmpd_include_dirs,
  install: true,
  version: libgmpd_version,
)

install_headers(libgmpd_headers, subdir: 'gmpd')

