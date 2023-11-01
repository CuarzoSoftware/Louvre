project(
    'Louvre',
    'c','cpp', 
    version : run_command('cat', files('../VERSION'), check : false).stdout().strip(),
    meson_version: '>= 0.56.0',
	  default_options: [
		'warning_level=2',
        'buildtype=release',
		'cpp_std=c++11',
	]
)

INCLUDES_PATH = '/usr/include/Louvre'
BACKENDS_PATH = '/usr/etc/Louvre/backends'
ASSETS_PATH = '/usr/etc/Louvre/assets'

cpp = meson.get_compiler('cpp')

include_paths = [
    include_directories('./lib'),
    include_directories('./lib/core')
]

include_paths_sys = [
    '/usr/include/drm',
    '/usr/include/libdrm',
    '/usr/include/pixman-1',
    '/usr/include/freetype2'
]

foreach p : include_paths_sys
    if run_command('[', '-d', p, ']', check : false).returncode() == 0
      include_paths += [include_directories(p)]
    endif
endforeach

# All headers
headers = run_command('find', './lib', '-type', 'f', '-name', '*.h', check : false).stdout().strip().split('\n')

# Public and private API headers
headers_classes = run_command('find', './lib/core', '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
headers_classes_private = run_command('find', './lib/core/private', '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
headers_other = run_command('find', './lib/other', '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
install_headers(headers_classes, install_dir : INCLUDES_PATH)
install_headers(headers_classes_private, install_dir : INCLUDES_PATH + '/private')
install_headers(headers_other, install_dir : INCLUDES_PATH + '/other')

globals = [
    'LinuxDMABuf',
	'Wayland',
	'XdgDecoration',
	'XdgShell',
    'WpPresentationTime'
]

foreach g : globals
    hs = run_command('find', './lib/protocols/' + g, '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(hs, install_dir : INCLUDES_PATH+'/protocols/' + g)
    
    hs_p = run_command('find', './lib/protocols/' + g + '/private', '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')

    if hs_p[0] != ''
    	install_headers(hs_p, install_dir : INCLUDES_PATH+'/protocols/' + g + '/private')
    endif
endforeach

wayland_server_dep  = cpp.find_library('wayland-server')
gl_dep              = cpp.find_library('GL')
egl_dep             = cpp.find_library('EGL')
glesv2_dep          = cpp.find_library('GLESv2')
udev_dep            = cpp.find_library('udev')
pthread_dep         = cpp.find_library('pthread')
xcursor_dep         = cpp.find_library('Xcursor')
xkbcommon_dep       = cpp.find_library('xkbcommon')
pixman_dep          = cpp.find_library('pixman-1')
dl_dep              = cpp.find_library('dl')
drm_dep             = cpp.find_library('drm')
gbm_dep             = cpp.find_library('gbm')
input_dep           = cpp.find_library('input')
libseat_dep         = cpp.find_library('seat')
freeimage_dep       = cpp.find_library('freeimage')
srm_dep             = cpp.find_library('SRM')

Louvre = library(
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
        freeimage_dep
    ],
    install : true)

meson.add_install_script('../scripts/ld.sh', get_option('prefix') + '/' + get_option('libdir'))

Louvre_dep = declare_dependency(
    dependencies: [], 
    include_directories : include_paths,
    link_with : Louvre)

subdir('backends/graphic/DRM')
subdir('backends/input/Libinput')

if get_option('build_examples')
    icuuc_dep = cpp.find_library('icuuc')
    fontconfig_dep = cpp.find_library('fontconfig')
    freetype_dep = cpp.find_library('freetype')

    subdir('examples/louvre-weston-clone')
    subdir('examples/louvre-default')
    subdir('examples/louvre-views')
endif