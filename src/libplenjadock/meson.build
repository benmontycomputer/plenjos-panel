libplenjadock_sources = [
  'plenjadock-interface.c',
  'plenjadock-application.c',
  'libplenjadock-blur.c'
]

cc = meson.get_compiler('c')

libplenjadock_deps = [
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('gtk+-x11-3.0', version: '>= 3.22'),
  dependency('libwnck-3.0'),
  cc.find_library('m', required : true),
]

libplenjadock = library('plenjadock', libplenjadock_sources, dependencies: libplenjadock_deps, install: true)
