libgmpd_dependencies = [
  dependency('glib-2.0'),
  dependency('gobject-2.0'),
  dependency('gio-2.0'),
  dependency('gio-unix-2.0'),
]

libgmpd_include_dirs = ['.']

libgmpd_version = '0.0.1'

libgmpd_sources = [
  'gmpd-audio-format.c',
  'gmpd-client.c',
  'gmpd-entity.c',
  'gmpd-entity-priv.h',
  'gmpd-error.c',
  'gmpd-object.c',
  'gmpd-object.h',
  'gmpd-response-priv.h',
  'gmpd-response.c',
  'gmpd-response.h',
  'gmpd-song.c',
  'gmpd-tag.c',
  'gmpd-version.c',
]

libgmpd_headers = [
  'gmpd.h',
  'gmpd-audio-format.h',
  'gmpd-client.h',
  'gmpd-entity.h',
  'gmpd-error.h',
  'gmpd-song.h',
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

