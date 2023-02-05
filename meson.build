project('42sh', 'c', version: '1.0',
    default_options: [
      'warning_level=3',
      'c_std=c99',
      'c_args=-D_POSIX_C_SOURCE=200809L'])


subdir('src/aliasmap')
subdir('src/ast')
subdir('src/builtin')
subdir('src/error_msg')
subdir('src/exec')
subdir('src/expansion')
subdir('src/funcmap')
subdir('src/io')
subdir('src/lexer')
subdir('src/log')
subdir('src/parser')
subdir('src/run')
subdir('src/string_t')
subdir('src/token')
subdir('src/varmap')
subdir('src/vector')

exe = executable(
  '42sh',
  files([ 'src/main.c' ]),
  dependencies : [
    aliasmap_dep,
    ast_dep,
    builtin_dep,
    error_msg_dep,
    exec_dep,
    expansion_dep,
    funcmap_dep,
    io_dep,
    lexer_dep,
    log_dep,
    parser_dep,
    run_dep,
    string_t_dep,
    token_dep,
    varmap_dep,
    vector_dep,
    ])
