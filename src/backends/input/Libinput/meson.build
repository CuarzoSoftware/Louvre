DRMBackend = library(
    'LInputBackendLibinput',
    sources : [
        'LInputBackendLibinput.cpp'
    ],
    include_directories : include_paths + [include_directories('./..')],
    dependencies : [
        louvre_dep,
        udev_dep,
	    input_dep,
        xkbcommon_dep,
        libseat_dep
    ],
    install : true, 
    install_dir : BACKENDS_INSTALL_PATH)

