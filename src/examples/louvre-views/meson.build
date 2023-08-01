sources = run_command('find', './src', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]').stdout().strip().split('\n')

executable(
    'louvre-views',
    sources,
    dependencies : [
        Louvre_dep,
        wayland_server_dep
    ],
    install : true, 
    install_dir : EXAMPLES_PATH)

assets = run_command('find', './assets', '-type', 'f', '-name', '*[.png]').stdout().strip().split('\n')

install_data(
    assets,
    install_dir: '/usr/etc/Louvre/assets'
)
