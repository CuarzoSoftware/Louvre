GraphicBackendDRM = library(
    'wayland',
    name_prefix : '',
    name_suffix : 'so',
    sources : [
        'LGraphicBackendWayland.cpp',
        '../../../lib/protocols/XdgShell/xdg-shell.c',
        '../../../lib/protocols/XdgDecoration/xdg-decoration-unstable-v1.c'
    ],
    include_directories : include_paths + [include_directories('./..')],
    dependencies : [
        louvre_dep,
        wayland_client_dep,
        wayland_egl_dep,
        egl_dep,
        gl_dep,
        srm_dep
    ],
    install : true,
    install_dir : join_paths(BACKENDS_INSTALL_PATH, 'graphic'))

