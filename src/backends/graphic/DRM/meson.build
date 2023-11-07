GraphicBackendDRM = library(
    'LGraphicBackendDRM',
    sources : [
        'LGraphicBackendDRM.cpp'
    ],
    include_directories : include_paths + [include_directories('./..')],
    dependencies : [
        Louvre_dep,
        udev_dep,
        egl_dep,
        gl_dep,
        drm_dep,
        gbm_dep,
        srm_dep
    ],
    install : true, 
    install_dir : BACKENDS_PATH)

