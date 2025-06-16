project(
    'cz-louvre',
    'c','cpp',
    version : run_command('cat', files('VERSION'), check : false).stdout().strip(),
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
VERSION_BUILD = run_command('cat', 'BUILD', check : false).stdout()

# -------------- CONFIGURATION --------------

HEADERS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('includedir'), 'CZ/Louvre')
BACKENDS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('libdir'), 'CZ/Louvre/Backends')
ASSETS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('datadir'), 'CZ/Louvre/Assets')

add_project_arguments([
    '-DLOUVRE_VERSION_MAJOR=@0@'.format(VERSION_MAJOR.to_int()),
    '-DLOUVRE_VERSION_MINOR=@0@'.format(VERSION_MINOR.to_int()),
    '-DLOUVRE_VERSION_PATCH=@0@'.format(VERSION_PATCH.to_int()),
    '-DLOUVRE_VERSION_BUILD=@0@'.format(VERSION_BUILD.to_int()),
    '-DLOUVRE_DEFAULT_BACKENDS_PATH="@0@"'.format(BACKENDS_INSTALL_PATH),
    '-DLOUVRE_DEFAULT_GRAPHIC_BACKEND="@0@"'.format(get_option('default_graphic_backend')),
    '-DLOUVRE_DEFAULT_INPUT_BACKEND="@0@"'.format(get_option('default_input_backend')),
    '-DLOUVRE_DEFAULT_ASSETS_PATH="@0@"'.format(ASSETS_INSTALL_PATH),
    '-Wno-missing-field-initializers'
], language: 'cpp')

# -------------- DEPENDENCIES --------------

pkg                 = import('pkgconfig')
cz_core_dep         = dependency('cz-core')
cz_skia_dep         = dependency('cz-skia')
wayland_server_dep  = dependency('wayland-server', version: '>= 1.20.0')
gl_dep              = dependency('gl', version: '>= 1.2')
egl_dep             = dependency('egl', version : '>=1.5')
glesv2_dep          = dependency('glesv2', version: '>= 3.2')
udev_dep            = dependency('libudev', version: '>= 249')
xcursor_dep         = dependency('xcursor', version: '>= 1.2.0')
xkbcommon_dep       = dependency('xkbcommon', version: '>= 1.4.0')
pixman_dep          = dependency('pixman-1', version: '>= 0.40.0')
drm_dep             = dependency('libdrm', version: '>= 2.4.113')
input_dep           = dependency('libinput', version: '>= 1.20.0')
libseat_dep         = dependency('libseat', version: '>= 0.6.4')
srm_dep             = dependency('SRM', version : '>=0.13.0')
pthread_dep         = cpp.find_library('pthread')
dl_dep              = cpp.find_library('dl')

# -------------- HEADERS --------------

include_paths = [include_directories('src/')]

# All headers
headers = run_command('find', 'src/CZ', '-type', 'f', '-name', '*.h', check : false).stdout().strip().split('\n')

# Public and private API headers
header_dirs = [
    ['src/CZ/Louvre', ''],
    ['src/CZ/Louvre/Roles', 'Roles'],
    ['src/CZ/Louvre/Private', 'Private'],
    ['src/CZ/Louvre/Other', 'Other'],
    ['src/CZ/Louvre/Events', 'Events']
]

foreach header_dir : header_dirs
    header_files = run_command('find', header_dir[0], '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(header_files, install_dir : join_paths(HEADERS_INSTALL_PATH, header_dir[1]))
    include_paths += include_directories(header_dir[0])
endforeach

protocols = [
    'LinuxDMABuf',
    'Wayland',
    'XdgActivation',
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
    'ImageCaptureSource',
    'LayerShell',
    'ForeignToplevelManagement',
    'ForeignToplevelList',
    'SinglePixelBuffer',
    'ContentType',
    'IdleNotify',
    'IdleInhibit',
    'DRMLease',
    'WlrOutputManagement',
    'SvgPath',
    'BackgroundBlur',
    'InvisibleRegion'
]

foreach p : protocols
    hs = run_command('find', 'src/CZ/Louvre/Protocols/' + p, '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(hs, install_dir : join_paths(HEADERS_INSTALL_PATH, 'Protocols', p))
endforeach

# -------------- LIBRARY --------------

louvre = library(
    'Louvre',
    sources : run_command('find', 'src/CZ', '-type', 'f', '-name', '*[.cpp,.c]', check : false).stdout().strip().split('\n'),
    include_directories : include_paths,
    dependencies : [
        cz_core_dep,
        cz_skia_dep,
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
        libseat_dep,
        pixman_dep
    ],
    soversion: VERSION_MAJOR,
    install : true)

louvre_dep = declare_dependency(
    dependencies: [cz_core_dep, cz_skia_dep, pixman_dep, wayland_server_dep, drm_dep, xkbcommon_dep],
    include_directories : include_paths,
    link_with : louvre)

pkg.generate(
    louvre,
    name: 'cz-louvre',
    description: 'C++ library for building Wayland compositors',
    version: meson.project_version(),
    subdirs: ['CZ/Louvre'],
    filebase: 'cz-louvre')

# -------------- SUBDIRS --------------

if get_option('backend-drm')
    subdir('src/backends/graphic/DRM')
endif

if get_option('backend-libinput')
    subdir('src/backends/input/Libinput')
endif

if get_option('backend-wayland-graphic') or get_option('backend-wayland-input')

    wayland_client_dep  = dependency('wayland-client', version: '>= 1.20.0')
    wayland_egl_dep     = dependency('wayland-egl', version: '>= 18.1.0')

    if get_option('backend-wayland-graphic')
        subdir('src/backends/graphic/Wayland')
    endif

    if get_option('backend-wayland-input')
        subdir('src/backends/input/Wayland')
    endif

endif

if get_option('build_examples')
    subdir('src/examples/louvre-default')
endif
