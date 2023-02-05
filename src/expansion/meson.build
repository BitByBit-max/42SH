expansion_src = files([
  'expansion.h',
  'expansion.c',
  ])

expansion_inc = include_directories('.')

expansion_lib = static_library('expansion', expansion_src)

expansion_dep = declare_dependency(
  link_with: expansion_lib,
  include_directories: expansion_inc,
  )
