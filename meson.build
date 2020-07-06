project(
  'kmc', 'c',
  version: '0.0.1',
  meson_version: '>= 0.50.0',
)

add_global_arguments(
  '-Wall',
  '-Wextra',
  '-D__GMPD_BUILD__',
  '-std=gnu11',
  language: 'c',
)

subdir('libgmpd')

