## libraries to link ##
@libs = qw(
  gdi32 ddraw msvcrt ole32 dinput dsound winmm
);
## end libraries to link ##

## library paths ##
if (defined($dxsdk_path)) {
  push (@lib_dirs, "$dxsdk_path/lib");
}
## end library paths ##

## statically shared objects ##
@common_objs = include_targets('common/targets.pl');
## end statically shared objects ##

## build game client ##
@client_objs = include_targets('client/targets.pl');
link_exe ('7kaa.exe', [@common_objs, @client_objs], \@libs, \@lib_dirs);
## end build game client ##

## build game server ##
if ($build_server) {
  @server_objs = include_targets('server/targets.pl');
  link_exe ('7kaa-server.exe', [@common_objs, @server_objs], \@libs, \@lib_dirs);
}
## end build game server ##
