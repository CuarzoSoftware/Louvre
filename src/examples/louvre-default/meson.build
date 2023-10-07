sources = run_command('find', '.', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]', check : true).stdout().strip().split('\n')

executable(
    'louvre-default',
    sources : sources,
    dependencies : [
        Louvre_dep,
        gl_dep,
    ],
    install : true)


