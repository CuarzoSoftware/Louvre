sources = run_command('find', '.', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]', check : false).stdout().strip().split('\n')

executable(
    'louvre-default',
    sources : sources,
    dependencies : [
        Louvre_dep,
        gl_dep,
    ],
    install : true)


