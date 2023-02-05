token_src = files([
  'token.h',
  'token.c',
  ])

token_inc = include_directories('.')

token_lib = static_library('token', token_src)

token_dep = declare_dependency(
  link_with: token_lib,
  include_directories: token_inc,
  )
