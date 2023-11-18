project(
    'LBenchmark',
    'c',
    version : '0.1.0',
    meson_version: '>= 0.56.0',
)
 
c = meson.get_compiler('c')
   
wayland_dep = c.find_library('wayland-client')
math_dep = c.find_library('m')
 
sources = [
    'main.c',
    'shm.c',
    'xdg-shell-protocol.c'
]
 
executable(
    'LBenchmark',
    sources : sources,
    dependencies : [
        wayland_dep,
        math_dep
])