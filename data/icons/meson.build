application_id = 'com.plenjos.Panel'

scalable_dir = join_paths('hicolor', 'scalable', 'apps')
install_data(
  join_paths(scalable_dir, ('@0@.svg').format(application_id)),
  install_dir: join_paths(get_option('datadir'), 'icons', scalable_dir)
)

symbolic_dir = join_paths('hicolor', 'symbolic', 'apps')
install_data(
  join_paths(symbolic_dir, ('@0@-symbolic.svg').format(application_id)),
  install_dir: join_paths(get_option('datadir'), 'icons', symbolic_dir)
)

#status_dir = join_paths('hicolor', '48x48', 'status')
#install_data(
#  join_paths(status_dir, ('plenjadock-app-indicator.png')),
#  install_dir: join_paths(get_option('datadir'), 'icons', status_dir)
#)
