parser_src = files([
  'parser.h',
  'parser.c',
  ])

parser_inc = include_directories('.')

parser_lib = static_library('parser', parser_src)

parser_dep = declare_dependency(
  link_with: parser_lib,
  include_directories: parser_inc,
  )
