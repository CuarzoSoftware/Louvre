GraphicBackendX11 = library(
    'LGraphicBackendX11',
    sources : [
        'LGraphicBackendX11.cpp'
    ],
    include_directories : include_paths + [include_directories('./..')],
    dependencies : [
        Louvre_dep,
        egl_dep,
        gl_dep,
        x11_dep,
        xfixes_dep,
        xrandr_dep
    ],
    install : true, 
    install_dir : BACKENDS_PATH)

