vector_src = files([
  'vector.h',
  'vector.c',
  ])

vector_inc = include_directories('.')

vector_lib = static_library('vector', vector_src)

vector_dep = declare_dependency(
  link_with: vector_lib,
  include_directories: vector_inc,
  )
