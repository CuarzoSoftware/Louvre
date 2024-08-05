project(
    'Louvre',
    'c','cpp',
    version : run_command('cat', files('../VERSION'), check : false).stdout().strip(),
    meson_version: '>= 0.56.0',
    default_options: [
        'warning_level=3',
        'buildtype=release',
        'cpp_std=c++20'
    ]
)

cpp = meson.get_compiler('cpp')

VERSION_MAJOR = meson.project_version().split('.')[0]
VERSION_MINOR = meson.project_version().split('.')[1]
VERSION_PATCH = meson.project_version().split('.')[2]
VERSION_BUILD = run_command('cat', '../BUILD', check : false).stdout()

# -------------- CONFIGURATION --------------

HEADERS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('includedir'), 'Louvre')
BACKENDS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('libdir'), 'Louvre/backends')
ASSETS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('datadir'), 'Louvre/assets')

add_project_arguments([
    '-DLOUVRE_VERSION_MAJOR=@0@'.format(VERSION_MAJOR.to_int()),
    '-DLOUVRE_VERSION_MINOR=@0@'.format(VERSION_MINOR.to_int()),
    '-DLOUVRE_VERSION_PATCH=@0@'.format(VERSION_PATCH.to_int()),
    '-DLOUVRE_VERSION_BUILD=@0@'.format(VERSION_BUILD.to_int()),
    '-DLOUVRE_DEFAULT_BACKENDS_PATH="@0@"'.format(BACKENDS_INSTALL_PATH),
    '-DLOUVRE_DEFAULT_GRAPHIC_BACKEND="@0@"'.format(get_option('default_graphic_backend')),
    '-DLOUVRE_DEFAULT_INPUT_BACKEND="@0@"'.format(get_option('default_input_backend')),
    '-DLOUVRE_DEFAULT_ASSETS_PATH="@0@"'.format(ASSETS_INSTALL_PATH),
    '-Wno-missing-field-initializers',
    '-Wlogical-op'
], language: 'cpp')

if get_option('buildtype') == 'custom'
    proj_args = ['-Ofast', '-finline-limit=1000', '-finline-functions', '-flto', '-funroll-loops', '-ffast-math', '-s', 
    '-march=native', '-fno-strict-aliasing', '-fdefer-pop', '-fmerge-constants', '-fthread-jumps',
    '-floop-optimize', '-fcrossjumping', '-fif-conversion', '-fguess-branch-probability']
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
srm_dep             = dependency('SRM', version : '>=0.7.0')
pthread_dep         = cpp.find_library('pthread')
dl_dep              = cpp.find_library('dl')

# -------------- HEADERS --------------

include_paths = [include_directories('./lib/')]

# All headers
headers = run_command('find', './lib', '-type', 'f', '-name', '*.h', check : false).stdout().strip().split('\n')

# Public and private API headers
header_dirs = [
    ['./lib/core', ''],
    ['./lib/core/private', 'private'],
    ['./lib/core/roles', ''],
    ['./lib/core/events', ''],
    ['./lib/core/scene', ''],
    ['./lib/core/framebuffers', ''],
    ['./lib/other', 'other'],
]

foreach header_dir : header_dirs
    header_files = run_command('find', header_dir[0], '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(header_files, install_dir : join_paths(HEADERS_INSTALL_PATH, header_dir[1]))
    include_paths += include_directories(header_dir[0])
endforeach

globals = [
    'LinuxDMABuf',
    'Wayland',
    'XdgDecoration',
    'XdgShell',
    'XdgOutput',
    'PresentationTime',
    'Viewporter',
    'FractionalScale',
    'GammaControl',
    'TearingControl',
    'RelativePointer',
    'PointerGestures',
    'SessionLock',
    'ScreenCopy',
    'LayerShell',
    'ForeignToplevelManagement',
    'SinglePixelBuffer',
    'ContentType',
    'IdleNotify',
    'IdleInhibit'
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
    sources : run_command('find', './lib', '-type', 'f', '-name', '*[.cpp,.c]', check : false).stdout().strip().split('\n'),
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
        pixman_dep
    ],
    soversion: VERSION_MAJOR,
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

if get_option('backend-drm')
    subdir('backends/graphic/DRM')
endif

if get_option('backend-libinput')
    subdir('backends/input/Libinput')
endif

if get_option('backend-wayland-graphic') or get_option('backend-wayland-input')

    wayland_client_dep  = dependency('wayland-client')
    wayland_egl_dep     = dependency('wayland-egl')

    if get_option('backend-wayland-graphic')
        subdir('backends/graphic/Wayland')
    endif

    if get_option('backend-wayland-input')
        subdir('backends/input/Wayland')
    endif

endif

if get_option('build_examples')
    fontconfig_dep = dependency('fontconfig')
    freetype_dep = dependency('freetype2')
    icuuc_dep = cpp.find_library('icuuc')
    subdir('examples')
endif

if get_option('build_tests')
    subdir('tests')
endif
