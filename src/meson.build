project(
    'Louvre',
    'c','cpp', 
    version : run_command('cat', files('../VERSION'),).stdout().strip() + '-'+ run_command('cat', files('../BUILD'),).stdout().strip(),
    meson_version: '>= 0.56.0',
	  default_options: [
		'warning_level=2',
		'buildtype=debug',
		'cpp_std=c++11',
	]
)

if get_option('buildtype') == 'custom'
    add_project_arguments('-Ofast', '-g3',  language : 'cpp')
endif

LIBRARY_PATH = '/usr/lib'
INCLUDES_PATH = '/usr/include/Louvre'
EXAMPLES_PATH = '/usr/bin'
BACKENDS_PATH = '/usr/etc/Louvre/backends'
 
cpp = meson.get_compiler('cpp')

include_paths = [
    include_directories('./lib'),
    include_directories('./lib/core')
]

include_paths_sys = [
    '/usr/local/include',
    '/usr/include/drm',
    '/usr/include/libdrm',
    '/usr/include/pixman-1',
    '/usr/include/freetype2'
]

foreach p : include_paths_sys
    if run_command('[', '-d', p, ']').returncode() == 0
      include_paths += [include_directories(p)]
    endif
endforeach

library_paths_all = [
    '/lib',
    '/lib64',
    '/usr/lib',
    '/usr/lib64',
    '/usr/local/lib',
    '/usr/local/lib64',

    # Multiarch support
    '/lib/x86_64-linux-gnu',
    '/usr/lib/x86_64-linux-gnu',
    '/usr/local/lib/x86_64-linux-gnu',
]

library_paths = []

foreach p : library_paths_all
    if run_command('[', '-d', p, ']').returncode() == 0
      library_paths += p
    endif
endforeach

# All headers
headers = run_command('find', './lib', '-type', 'f', '-name', '*.h').stdout().strip().split('\n')

# Public and private API headers
headers_classes = run_command('find', './lib/core', '-type', 'f', '-name', '*.h', '-maxdepth', '1').stdout().strip().split('\n')
headers_classes_private = run_command('find', './lib/core/private', '-type', 'f', '-name', '*.h', '-maxdepth', '1').stdout().strip().split('\n')
headers_other = run_command('find', './lib/other', '-type', 'f', '-name', '*.h', '-maxdepth', '1').stdout().strip().split('\n')
install_headers(headers_classes, install_dir : INCLUDES_PATH)
install_headers(headers_classes_private, install_dir : INCLUDES_PATH+'/private')
install_headers(headers_other, install_dir : INCLUDES_PATH+'/other')

globals = [
        'LinuxDMABuf',
	'Wayland',
	'XdgDecoration',
	'XdgShell',
        'WpPresentationTime'
]

foreach g : globals
    hs = run_command('find', './lib/protocols/' + g, '-type', 'f', '-name', '*.h', '-maxdepth', '1').stdout().strip().split('\n')
    install_headers(hs, install_dir : INCLUDES_PATH+'/protocols/' + g)
    
    hs_p = run_command('find', './lib/protocols/' + g + '/private', '-type', 'f', '-name', '*.h', '-maxdepth', '1').stdout().strip().split('\n')

    if hs_p[0] != ''
    	install_headers(hs_p, install_dir : INCLUDES_PATH+'/protocols/' + g + '/private')
    endif
endforeach


wayland_server_dep  = cpp.find_library('wayland-server', dirs: library_paths, required: true)
egl_dep             = cpp.find_library('EGL', dirs: library_paths, required: true)
glesv2_dep          = cpp.find_library('GLESv2', dirs: library_paths, required: true)
udev_dep            = cpp.find_library('udev', dirs: library_paths, required: true)
pthread_dep         = cpp.find_library('pthread', dirs: library_paths, required: true)
xcursor_dep         = cpp.find_library('Xcursor', dirs: library_paths, required: true)
xkbcommon_dep       = cpp.find_library('xkbcommon', dirs: library_paths, required: true)
pixman_dep          = cpp.find_library('pixman-1', dirs: library_paths, required: true)
dl_dep              = cpp.find_library('dl', dirs: library_paths, required: true)
drm_dep             = cpp.find_library('drm', dirs: library_paths, required: true)
gbm_dep             = cpp.find_library('gbm', dirs: library_paths, required: true)
input_dep           = cpp.find_library('input', dirs: library_paths, required: true)
libseat_dep         = cpp.find_library('libseat', dirs: library_paths, required: true)
freeimage_dep       = cpp.find_library('freeimage', dirs: library_paths, required: true)
srm_dep             = cpp.find_library('SRM', dirs: library_paths, required: true)


if get_option('build_drm_backend') or get_option('build_x11_backend')
    gl_dep = cpp.find_library('GL', dirs: library_paths, required: true)
endif

if get_option('build_x11_backend')
    x11_dep = cpp.find_library('X11', dirs: library_paths, required: true)
    xfixes_dep = cpp.find_library('Xfixes', dirs: library_paths, required: true)
    xrandr_dep = cpp.find_library('Xrandr', dirs: library_paths, required: true)
endif

Louvre = library(
    'Louvre',
    sources : run_command('find', './lib', '-type', 'f', '-name', '*[.cpp,.c]').stdout().strip().split('\n'),
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
    install : true, 
    install_dir : LIBRARY_PATH)

Louvre_dep = declare_dependency(
    dependencies: [], 
    include_directories : include_paths,
    link_with : Louvre)


if get_option('build_drm_backend')
    subdir('backends/graphic/DRM')
endif

if get_option('build_x11_backend')
    #subdir('backends/graphic/X11')
endif

subdir('backends/input/Libinput')
#subdir('backends/input/X11')

if get_option('build_examples')
    #subdir('examples/louvre-weston-clone')
    #subdir('examples/louvre-default')
    subdir('examples/louvre-views')
endif

