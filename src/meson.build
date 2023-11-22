project(
    'Louvre',
    'c','cpp', 
    version : run_command('cat', files('../VERSION'), check : false).stdout().strip(),
    meson_version: '>= 0.56.0',
    default_options: [
        'warning_level=2',
        'buildtype=release',
        'cpp_std=c++14'
	]
)

cpp = meson.get_compiler('cpp')

# -------------- CONFIGURATION --------------

HEADERS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('includedir'), 'Louvre')
BACKENDS_INSTALL_PATH = get_option('backends_path')
ASSETS_INSTALL_PATH = get_option('assets_path')

configuration_data = configuration_data()
configuration_data.set('LOUVRE_DEFAULT_BACKENDS_PATH', BACKENDS_INSTALL_PATH)
configuration_data.set('LOUVRE_DEFAULT_ASSETS_PATH', ASSETS_INSTALL_PATH)

config_header = configure_file(
  input : './lib/core/LConfig.h.in',
  output : 'LConfig.h',
  configuration : configuration_data,
  install : true,
  install_dir : HEADERS_INSTALL_PATH 
)

if get_option('buildtype') == 'custom'
    proj_args = ['-Ofast', '-s', '-march=native', '-fno-strict-aliasing']
    add_project_arguments(proj_args, language : 'c')
    add_project_arguments(proj_args, language : 'cpp')
endif

# -------------- DEPENDENCIES --------------

pkg                 = import('pkgconfig')
wayland_server_dep  = dependency('wayland-server')
gl_dep              = dependency('gl')
egl_dep             = dependency('egl')
glesv2_dep          = dependency('glesv2')
udev_dep            = dependency('libudev')
xcursor_dep         = dependency('xcursor')
xkbcommon_dep       = dependency('xkbcommon')
pixman_dep          = dependency('pixman-1')
drm_dep             = dependency('libdrm')
gbm_dep             = dependency('gbm')
input_dep           = dependency('libinput')
libseat_dep         = dependency('libseat')
srm_dep             = dependency('SRM')
pthread_dep         = cpp.find_library('pthread')
freeimage_dep       = cpp.find_library('freeimage')
dl_dep              = cpp.find_library('dl')

# -------------- HEADERS --------------

include_paths = [
    include_directories('./lib'),
    include_directories('./lib/core')
]

# All headers
headers = run_command('find', './lib', '-type', 'f', '-name', '*.h', check : false).stdout().strip().split('\n')

# Public and private API headers
headers_classes = run_command('find', './lib/core', '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
headers_classes_private = run_command('find', './lib/core/private', '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
headers_other = run_command('find', './lib/other', '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
install_headers(headers_classes, install_dir : HEADERS_INSTALL_PATH)
install_headers(headers_classes_private, install_dir : join_paths(HEADERS_INSTALL_PATH, 'private'))
install_headers(headers_other, install_dir : join_paths(HEADERS_INSTALL_PATH, 'other'))

globals = [
    'LinuxDMABuf',
	'Wayland',
	'XdgDecoration',
	'XdgShell',
    'WpPresentationTime'
]

foreach g : globals
    hs = run_command('find', './lib/protocols/' + g, '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(hs, install_dir : join_paths(HEADERS_INSTALL_PATH, 'protocols', g))
    
    hs_p = run_command('find', './lib/protocols/' + g + '/private', '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')

    if hs_p[0] != ''
    	install_headers(hs_p, install_dir : join_paths(HEADERS_INSTALL_PATH, 'protocols', g, 'private'))
    endif
endforeach

# -------------- LIBRARY --------------

louvre = library(
    'Louvre',
    sources : [run_command('find', './lib', '-type', 'f', '-name', '*[.cpp,.c]', check : false).stdout().strip().split('\n'), config_header],
    include_directories : include_paths,
    dependencies : [
        wayland_server_dep,
        egl_dep,
        glesv2_dep,
        input_dep,
        udev_dep,
        pthread_dep,
        xcursor_dep,
        xkbcommon_dep,
        pixman_dep,
        dl_dep,
        drm_dep,
        gbm_dep,
        libseat_dep,
        freeimage_dep,
        pixman_dep
    ],
    install : true)

louvre_dep = declare_dependency(
    dependencies: [pixman_dep, wayland_server_dep, drm_dep, xkbcommon_dep],
    include_directories : include_paths,
    link_with : louvre)

pkg.generate(
    louvre,
    name: 'Louvre',
    description: 'C++ library for building Wayland compositors',
    version: meson.project_version(),
    subdirs: ['Louvre'],
    filebase: 'Louvre')

# -------------- SUBDIRS --------------

subdir('backends/graphic/DRM')
subdir('backends/input/Libinput')

if get_option('build_examples')
    fontconfig_dep = dependency('fontconfig')
    freetype_dep = dependency('freetype2')
    icuuc_dep = cpp.find_library('icuuc')

    subdir('examples/louvre-weston-clone')
    subdir('examples/louvre-default')
    subdir('examples/louvre-views')
endif
