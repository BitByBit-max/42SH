error_msg_src = files([
  'error_msg.h',
  'error_msg.c',
  ])

error_msg_inc = include_directories('.')

error_msg_lib = static_library('error_msg', error_msg_src)

error_msg_dep = declare_dependency(
  link_with: error_msg_lib,
  include_directories: error_msg_inc,
  )
