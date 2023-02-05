string_t_src = files([
  'string_t.h',
  'string_t.c',
  ])

string_t_inc = include_directories('.')

string_t_lib = static_library('string_t', string_t_src)

string_t_dep = declare_dependency(
  link_with: string_t_lib,
  include_directories: string_t_inc,
  )
