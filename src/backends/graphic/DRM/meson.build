GraphicBackendDRM = library(
    'drm',
    name_prefix : '',
    name_suffix : 'so',
    sources : [
        'LGraphicBackendDRM.cpp'
    ],
    include_directories : include_paths + [include_directories('./..')],
    dependencies : [
        louvre_dep,
        udev_dep,
        egl_dep,
        gl_dep,
        drm_dep,
        gbm_dep,
        srm_dep
    ],
    install : true, 
    install_dir : join_paths(BACKENDS_INSTALL_PATH, 'graphic'))

