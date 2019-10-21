project('libsplit', 'c')

libsplit_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22')
]

incdir = include_directories('include')

libsplit = shared_library('split',
  'spl-tile-manager.c',
  'gtk/spl-workspace.c',
  dependencies : libsplit_deps,
  include_directories : incdir,
  install : true)

libsplit_dep = declare_dependency(include_directories : incdir,
link_with : libsplit)
