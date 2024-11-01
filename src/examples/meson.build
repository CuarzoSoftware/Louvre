# Textures, icons, etc
assets = run_command('find', './common/assets', '-type', 'f', '-name', '*.png', check : false).stdout().strip().split('\n')

# Wayland session
session_conf = configuration_data()
session_conf.set('LOUVRE_VIEWS_PATH', join_paths(get_option('prefix'), get_option('bindir'), 'louvre-views'))
configure_file(input : './common/assets/louvre.desktop.in',
               output : 'louvre.desktop',
               configuration : session_conf,
               install_dir : join_paths(get_option('prefix'), '/usr/local/share/wayland-sessions'))

# Default dock configuration
apps_conf = configuration_data()
apps_conf.set('ASSETS_PATH', ASSETS_INSTALL_PATH)
configure_file(input : './common/assets/apps.list.in',
               output : 'apps.list',
               configuration : apps_conf,
               install_dir : ASSETS_INSTALL_PATH)
install_data(
    assets,
    install_dir: ASSETS_INSTALL_PATH
)

# Common source files
common_src = files('./common/TextRenderer.cpp')

subdir('louvre-default')
subdir('louvre-weston-clone')
subdir('louvre-views')
