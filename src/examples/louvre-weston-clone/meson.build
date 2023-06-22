sources = run_command('find', '.', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]').stdout().strip().split('\n')

fontconfig_dep = cpp.find_library('fontconfig', dirs: library_paths, required: true)
freetype_dep = cpp.find_library('freetype', dirs: library_paths, required: true)

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
    install : true, 
    install_dir : EXAMPLES_PATH)

#wallpaper = 'louvre-wallpaper.png'
#configure_file(
#    input : wallpaper,
#    output : wallpaper,
#    copy : true)
