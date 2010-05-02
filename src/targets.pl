## libraries to link ##
@dxlibs = qw(
  dinput
);
@libs = qw(
  ole32 msvcrt winmm
);
## end libraries to link ##

## library paths ##
if (defined($dxsdk_path)) {
  push (@lib_dirs, "$dxsdk_path/lib");
}
## end library paths ##

## Build audio backend ##
if (defined($audio_backend)) {
  if ($audio_backend =~ /OpenAL/i) {
    if ($platform =~ /^linux/) {
      push (@libs, "openal");
    } elsif ($platform =~ /^win32/) {
      push (@libs, "openal32");
    }
    @audio = include_targets('audio/openal/targets.pl');
  } elsif ($audio_backend =~ /dsound/i) {
    push (@libs, 'dsound');
    @audio = include_targets('audio/dsound/targets.pl');
  }
}
## Done building the audio backend ##

## Build the video backend ##
push (@libs, 'gdi32', 'ddraw');
@video = include_targets('video/targets.pl');
## Done building the vidio backend ##

## statically shared objects ##
@common_objs = include_targets('common/targets.pl');
@input = include_targets('input/dinput/targets.pl');
@imgfun = include_targets('imgfun/targets.pl');
## end statically shared objects ##

## build game client ##
@client_objs = include_targets('client/targets.pl');
link_exe ('7kaa.exe',
          [@common_objs, @audio, @input, @video, @imgfun, @client_objs],
          [@libs, @dxlibs],
          \@lib_dirs);
## end build game client ##

## build game server ##
if ($build_server) {
  @server_objs = include_targets('server/targets.pl');
  link_exe ('7kaa-server.exe',
            [@common_objs, @audio, @input, @video, @imgfun, @server_objs],
            [@libs, @dxlibs],
            \@lib_dirs);
}
## end build game server ##
