bluedit_sources = [
  'main.c',
  'bluedit-window.c',
  'bl-multi-editor.c',
  'views/bl-editor.c',
  'bl-document.c',
  'bl-markdown-view.c',
  'bl-workspace.c',
  'views/bl-view.c'
]

bluedit_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('libcmark'),
  libsplit_dep # defined in top level meson.build
]

gnome = import('gnome')

bluedit_sources += gnome.compile_resources('bluedit-resources',
  'res/bluedit.gresource.xml',
  source_dir: 'res',
  c_name: 'bluedit'
)

executable('bluedit', bluedit_sources,
  dependencies: bluedit_deps,
  install: true,
)
