run_src = files([
  'run.h',
  'run.c',
  ])

run_inc = include_directories('.')

run_lib = static_library('run', run_src)

run_dep = declare_dependency(
  link_with: run_lib,
  include_directories: run_inc,
  )
