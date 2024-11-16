DRMBackend = library(
    'libinput',
    name_prefix : '',
    name_suffix : 'so',
    sources : [
        louvre_extra_files,
        'LInputBackendLibinput.cpp'
    ],
    include_directories : include_paths + [include_directories('./..')],
    dependencies : [
        partial_deps,
        louvre_dep,
        udev_dep,
	    input_dep,
        xkbcommon_dep,
        libseat_dep
    ],
    install : true,
    install_dir : join_paths(BACKENDS_INSTALL_PATH, 'input'))

