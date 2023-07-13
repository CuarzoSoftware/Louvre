sources = run_command('find', '.', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]').stdout().strip().split('\n')

executable(
    'louvre-views',
    sources,
    dependencies : [
        Louvre_dep,
        wayland_server_dep
    ],
    install : true, 
    install_dir : EXAMPLES_PATH)
