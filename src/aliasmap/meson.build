aliasmap_src = files([
  'aliasmap.h',
  'aliasmap.c',
  ])

aliasmap_inc = include_directories('.')

aliasmap_lib = static_library('aliasmap', aliasmap_src)

aliasmap_dep = declare_dependency(
  link_with: aliasmap_lib,
  include_directories: aliasmap_inc,
  )
