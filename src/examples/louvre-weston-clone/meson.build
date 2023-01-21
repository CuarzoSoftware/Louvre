sources = run_command('find', '.', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]').stdout().strip().split('\n')

executable(
    'louvre-weston-clone',
    sources : sources,
    dependencies : [
        Louvre_dep,
        gl_dep,
    ],
    install : true, 
    install_dir : EXAMPLES_PATH)

#wallpaper = 'louvre-wallpaper.png'
#configure_file(
#    input : wallpaper,
#    output : wallpaper,
#    copy : true)
