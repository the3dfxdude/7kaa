### Directories to build in ###
include_targets(qw( common/targets.pl client/targets.pl ));
if ($build_server) {
  include_targets(qw( server/targets.pl ));
}
