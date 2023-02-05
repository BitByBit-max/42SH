log_src = files([
  'log.h',
  'log.c',
  ])

log_inc = include_directories('.')

log_lib = static_library('log', log_src)

log_dep = declare_dependency(
  link_with: log_lib,
  include_directories: log_inc,
  )
