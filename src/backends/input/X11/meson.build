DRMBackend = library(
    'LInputBackendX11',
    sources : [
        'LInputBackendX11.cpp'
    ],
    include_directories : include_paths + [include_directories('./..')],
    dependencies : [
        Louvre_dep,
	x11_dep,
	xkbcommon_dep
    ],
    install : true, 
    install_dir : BACKENDS_PATH)

