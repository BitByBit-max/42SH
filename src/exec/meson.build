exec_src = files([
  'exec.h',
  'exec.c',
  ])

exec_inc = include_directories('.')

exec_lib = static_library('exec', exec_src)

exec_dep = declare_dependency(
  link_with: exec_lib,
  include_directories: exec_inc,
  )
