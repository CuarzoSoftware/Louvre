sources = run_command('find', './src', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]', check : false).stdout().strip().split('\n')

executable(
    'louvre-views',
    [sources, common_src],
    dependencies : [
        louvre_dep,
        wayland_server_dep,
        input_dep,
        fontconfig_dep,
        freetype_dep,
        icuuc_dep
    ],
    install : true)