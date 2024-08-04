subdir('libplenjadock')

panel_sources = [
  'main.c',
  'panel-window.c',
  'panel-applications-menu.c',
]

panel_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('libwnck-3.0'),
  dependency('libhandy-1', version: '>= 1.0.0'),
  #dependency('clutter-gtk-1.0'),
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
