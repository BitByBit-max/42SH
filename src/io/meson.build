io_src = files([
  'io.h',
  'io.c',
  ])

io_inc = include_directories('.')

io_lib = static_library('io', io_src)

io_dep = declare_dependency(
  link_with: io_lib,
  include_directories: io_inc,
  )
