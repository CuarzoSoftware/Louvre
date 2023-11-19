sources = run_command('find', '.', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]', check : false).stdout().strip().split('\n')

config_header = configure_file(
  input : '../../lib/core/LConfig.h.in',
  output : 'LConfig.h',
  configuration : configuration_data,
  install : false
)

executable(
    'louvre-weston-clone',
    sources,
    dependencies : [
        louvre_dep,
        wayland_server_dep,
        gl_dep,
        fontconfig_dep,
        freetype_dep
    ],
    install : true)
