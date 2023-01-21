GraphicBackendDRM = library(
    'LGraphicBackendDRM',
    sources : [
        'LGraphicBackendDRM.cpp',
        'LDRM.cpp',
        'LDRMConnector.cpp',
        'LDRMDevice.cpp',
        'LDRMCrtc.cpp',
        'LDRMEncoder.cpp',
        'LDRMOutput.cpp',
        'LDRMOutputMode.cpp',
        'LDRMPlane.cpp'
    ],
    include_directories : include_paths + [include_directories('./..')],
    dependencies : [
        Louvre_dep,
        udev_dep,
        egl_dep,
        gl_dep,
        drm_dep,
        gbm_dep
    ],
    install : true, 
    install_dir : BACKENDS_PATH)

