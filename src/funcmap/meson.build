funcmap_src = files([
  'funcmap.h',
  'funcmap.c',
  ])

funcmap_inc = include_directories('.')

funcmap_lib = static_library('funcmap', funcmap_src)

funcmap_dep = declare_dependency(
  link_with: funcmap_lib,
  include_directories: funcmap_inc,
  )
