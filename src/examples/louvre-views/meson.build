sources = run_command('find', './src', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]', check : false).stdout().strip().split('\n')

executable(
    'louvre-views',
    sources,
    dependencies : [
        louvre_dep,
        wayland_server_dep,
        input_dep,
        fontconfig_dep,
        freetype_dep,
        icuuc_dep
    ],
    install : true)

assets = run_command('find', './assets', '-type', 'f', '-name', '*[.png, .list]', check : false).stdout().strip().split('\n')

install_data(
    assets,
    install_dir: ASSETS_INSTALL_PATH
)
