DRMBackend = library(
    'wayland',
    name_prefix : '',
    name_suffix : 'so',
    sources : [
        'LInputBackendWayland.cpp'
    ],
    include_directories : include_paths + [include_directories('./..')],
    dependencies : [
        louvre_dep,
        wayland_client_dep
    ],
    install : true,
    install_dir : join_paths(BACKENDS_INSTALL_PATH, 'input'))

