DRMBackend = library(
    'LInputBackendLibinput',
    sources : [
        'LInputBackendLibinput.cpp'
    ],
    include_directories : include_paths + [include_directories('./..')],
    dependencies : [
        Louvre_dep,
        udev_dep,
	    input_dep,
        xkbcommon_dep,
        libseat_dep
    ],
    install : true, 
    install_dir : BACKENDS_PATH)

