### Directories to build in ###
@dirs = qw( common client );
if ($build_server) {
  push (@dirs, 'server');
}
