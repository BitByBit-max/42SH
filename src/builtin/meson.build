builtin_src = files([
  'alias.c',
  'builtin.h',
  'builtin.c',
  'cd.c',
  'dot.c',
  'echo.c',
  'exit.c',
  'export.c',
  'unalias.c',
  'unset.c'
  ])

builtin_inc = include_directories('.')

builtin_lib = static_library('builtin', builtin_src)

builtin_dep = declare_dependency(
  link_with: builtin_lib,
  include_directories: builtin_inc,
  )
