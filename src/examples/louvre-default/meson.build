sources = run_command('find', '.', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]', check : false).stdout().strip().split('\n')

executable(
    'louvre-default',
    sources : sources,
    dependencies : [
        louvre_dep,
        gl_dep,
    ],
    install : true)


