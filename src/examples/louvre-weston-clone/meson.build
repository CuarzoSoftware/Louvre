sources = run_command('find', './src', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]', check : false).stdout().strip().split('\n')

executable(
    'louvre-weston-clone',
    [louvre_extra_files, sources, common_src],
    dependencies : [
        partial_deps,
        louvre_dep,
        gl_dep,
        fontconfig_dep,
        freetype_dep,
        icuuc_dep
    ],
    install : true)
