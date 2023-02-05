ast_src = files([
  'ast.h',
  'ast_init.c',
  'ast_destroy.c',
  'ast_print.c',
  'ast_dup.c'
  ])

ast_inc = include_directories('.')

ast_lib = static_library('ast', ast_src)

ast_dep = declare_dependency(
  link_with: ast_lib,
  include_directories: ast_inc,
  )
