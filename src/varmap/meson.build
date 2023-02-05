varmap_src = files([
  'varmap.h',
  'varmap.c',
  'special_vars.c',
  'special_vars.h',
  ])

varmap_inc = include_directories('.')

varmap_lib = static_library('varmap', varmap_src)

varmap_dep = declare_dependency(
  link_with: varmap_lib,
  include_directories: varmap_inc,
  )
