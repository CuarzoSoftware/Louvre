sources = run_command('find', '.', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]', check : false).stdout().strip().split('\n')

executable(
    'louvre-weston-clone',
    sources : sources,
    dependencies : [
        louvre_dep,
        wayland_server_dep,
        gl_dep,
        fontconfig_dep,
        freetype_dep
    ],
    install : true)
