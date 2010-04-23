## libraries to link ##
@dxlibs = qw(
  gdi32 ddraw dsound dinput
);
@libs = qw(
  ole32 msvcrt winmm
);
if (defined($audio_backend) && $audio_backend eq "OpenAL") {
  if ($platform =~ /^linux/) {
    push (@libs, "openal");
  } elsif ($platform =~ /^win32/) {
    push (@libs, "openal32");
  }
}
## end libraries to link ##

## library paths ##
if (defined($dxsdk_path)) {
  push (@lib_dirs, "$dxsdk_path/lib");
}
## end library paths ##

## statically shared objects ##
@common_objs = include_targets('common/targets.pl');
#@nogui = include_targets('gui/none/targets.pl');
@win32gui = include_targets('gui/win32/targets.pl');
@anygui = include_targets('gui/any/targets.pl');
@imgfun = include_targets('imgfun/targets.pl');
## end statically shared objects ##

## build game client ##
@client_objs = include_targets('client/targets.pl');
link_exe ('7kaa.exe',
          [@common_objs, @anygui, @win32gui, @imgfun, @client_objs],
          [@libs, @dxlibs],
          \@lib_dirs);
## end build game client ##

## build game server ##
if ($build_server) {
  @server_objs = include_targets('server/targets.pl');
  link_exe ('7kaa-server.exe',
            [@common_objs, @anygui, @win32gui, @imgfun, @server_objs],
            [@libs, @dxlibs],
            \@lib_dirs);
}
## end build game server ##
