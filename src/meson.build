subdir('libplenjadock')

panel_sources = [
  'main.c',
  'panel-window.c',
  'panel-applications-menu.c',
]

panel_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
]

gnome = import('gnome')

panel_sources += gnome.compile_resources('panel-resources',
  'panel.gresource.xml',
  c_name: 'panel'
)

executable('plenjos-panel', panel_sources,
  dependencies: panel_deps,
  link_with: libplenjadock,
  install: true,
)
