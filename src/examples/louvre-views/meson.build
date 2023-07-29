sources = run_command('find', '.', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]').stdout().strip().split('\n')

executable(
    'louvre-views',
    sources,
    dependencies : [
        Louvre_dep,
        wayland_server_dep
    ],
    install : true, 
    install_dir : EXAMPLES_PATH)

install_data(
    files('assets/dock_side.png',
    'assets/dock_clamp.png',
    'assets/toplevel_active_lower_clamp.png',
    'assets/toplevel_active_lower_corner.png',
    'assets/toplevel_active_side_clamp.png',
    'assets/toplevel_active_topbar_clamp.png',
    'assets/toplevel_active_upper_corner.png',

    'assets/toplevel_inactive_lower_clamp.png',
    'assets/toplevel_inactive_lower_corner.png',
    'assets/toplevel_inactive_side_clamp.png',
    'assets/toplevel_inactive_topbar_clamp.png',
    'assets/toplevel_inactive_upper_corner.png',

    'assets/toplevel_border_radius_mask.png'),
    install_dir: '/usr/etc/Louvre/assets'
)
