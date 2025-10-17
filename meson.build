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
NAME='Louvre'
VERSION_MAJOR = meson.project_version().split('.')[0]
VERSION_MINOR = meson.project_version().split('.')[1]
VERSION_PATCH = meson.project_version().split('.')[2]
VERSION_BUILD = run_command('cat', './BUILD', check : false).stdout()
HEADERS_INSTALL_DIR = join_paths(get_option('prefix'), get_option('includedir'), 'CZ/', NAME)
CONF_FILE_NAME = 'CZ' + NAME + 'Version.h'

conf = configuration_data()
conf.set('VERSION_MAJOR', VERSION_MAJOR)
conf.set('VERSION_MINOR', VERSION_MINOR)
conf.set('VERSION_PATCH', VERSION_PATCH)
conf_file = configure_file(
    input : 'src/CZ/' + NAME + '/' + CONF_FILE_NAME +'.in',
    output : CONF_FILE_NAME,
    configuration : conf)

install_data(
    conf_file,
    install_dir: HEADERS_INSTALL_DIR)

# -------------- DEPENDENCIES --------------

pkg                 = import('pkgconfig')
cz_core_dep         = dependency('cz-core')
cz_skia_dep         = dependency('cz-skia')
cz_srm_dep          = dependency('cz-srm', fallback:['cz-srm', 'cz_srm_dep'])
cz_ream_dep         = dependency('cz-ream')
wayland_server_dep  = dependency('wayland-server', version: '>= 1.20.0')
wayland_client_dep  = dependency('wayland-client')
gl_dep              = dependency('gl', version: '>= 1.2')
egl_dep             = dependency('egl', version : '>=1.5')
glesv2_dep          = dependency('glesv2', version: '>= 3.2')
udev_dep            = dependency('libudev', version: '>= 249')
xcursor_dep         = dependency('xcursor', version: '>= 1.2.0')
xkbcommon_dep       = dependency('xkbcommon', version: '>= 1.4.0')
drm_dep             = dependency('libdrm', version: '>= 2.4.113')
input_dep           = dependency('libinput', version: '>= 1.20.0')
libseat_dep         = dependency('libseat', version: '>= 0.6.4')
pthread_dep         = cpp.find_library('pthread')
dl_dep              = cpp.find_library('dl')

# -------------- HEADERS --------------

include_paths = ['src/', 'src/CZ']

# All headers
headers = run_command('find', 'src/CZ', '-type', 'f', '-name', '*.h', check : false).stdout().strip().split('\n')

# Public and private API headers
header_dirs = [
    ['src/CZ/Louvre', ''],
    ['src/CZ/Louvre/Events', 'Events'],
    ['src/CZ/Louvre/Roles', 'Roles'],
    ['src/CZ/Louvre/Private', 'Private'],
    ['src/CZ/Louvre/Other', 'Other'],
    ['src/CZ/Louvre/Cursor', 'Cursor'],
    ['src/CZ/Louvre/Layout', 'Layout'],
    ['src/CZ/Louvre/Manager', 'Manager'],
    ['src/CZ/Louvre/Seat', 'Seat'],
    ['src/CZ/Louvre/Backends', 'Backends'],
    ['src/CZ/Louvre/Backends/DRM', 'Backends/DRM'],
    ['src/CZ/Louvre/Backends/Wayland', 'Backends/Wayland'],
    ['src/CZ/Louvre/Backends/Offscreen', 'Backends/Offscreen']
]

foreach header_dir : header_dirs
    header_files = run_command('find', header_dir[0], '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(header_files, install_dir : join_paths(HEADERS_INSTALL_DIR, header_dir[1]))
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
    'InvisibleRegion',
    'CursorShape',
    'Tablet',
    'WaylandDRM',
    'DRMSyncObj'
]

foreach p : protocols
    hs = run_command('find', 'src/CZ/Louvre/Protocols/' + p, '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(hs, install_dir : join_paths(HEADERS_INSTALL_DIR, 'Protocols', p))
endforeach

# -------------- LIBRARY --------------

deps = [
    cz_core_dep,
    cz_skia_dep,
    cz_srm_dep,
    cz_ream_dep,
    wayland_server_dep,
    wayland_client_dep,
    egl_dep,
    glesv2_dep,
    input_dep,
    udev_dep,
    pthread_dep,
    xcursor_dep,
    xkbcommon_dep,
    dl_dep,
    drm_dep,
    libseat_dep,
]

cz_louvre = library(
    'cz-louvre',
    sources : run_command('find', 'src/CZ', '-type', 'f', '-name', '*[.cpp,.c]', check : false).stdout().strip().split('\n'),
    include_directories : include_paths,
    dependencies : deps,
    soversion: VERSION_MAJOR,
    install : true)

cz_louvre_dep = declare_dependency(
    dependencies: deps,
    include_directories : include_paths,
    link_with : cz_louvre)

pkg.generate(
    cz_louvre,
    name: 'cz-louvre',
    description: 'C++ library for building Wayland compositors',
    version: meson.project_version(),
    libraries: deps,
    subdirs: ['CZ','CZ/Louvre'],
    requires: ['cz-core'],
    filebase: 'cz-louvre')

# -------------- SUBDIRS --------------

if get_option('build_examples')
    subdir('src/examples/cz-louvre-default')
endif
