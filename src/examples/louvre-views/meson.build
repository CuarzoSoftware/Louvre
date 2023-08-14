sources = run_command('find', './src', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]').stdout().strip().split('\n')

fontconfig_dep = cpp.find_library('fontconfig', dirs: library_paths, required: true)
freetype_dep = cpp.find_library('freetype', dirs: library_paths, required: true)

executable(
    'louvre-views',
    sources,
    dependencies : [
        Louvre_dep,
        wayland_server_dep,
        input_dep,
        fontconfig_dep,
        freetype_dep,
        icuuc_dep
    ],
    install : true, 
    install_dir : EXAMPLES_PATH)

assets = run_command('find', './assets', '-type', 'f', '-name', '*[.png]').stdout().strip().split('\n')

install_data(
    assets,
    install_dir: '/usr/etc/Louvre/assets'
)
