sources = run_command('find', '.', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]', check : true).stdout().strip().split('\n')

executable(
    'louvre-weston-clone',
    sources : sources,
    dependencies : [
        Louvre_dep,
        wayland_server_dep,
        gl_dep,
        fontconfig_dep,
        freetype_dep
    ],
    install : true)
