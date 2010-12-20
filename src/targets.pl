## libraries to link ##
my @libs;
unless ($disable_wine) {
  push (@libs, 'ole32','msvcrt','winmm');
}
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
if (defined($video_backend)) {
  if ($video_backend =~ /sdl/i) {
    push (@libs, 'SDL');
    @video = include_targets('video/sdl/targets.pl');
  } elsif ($video_backend =~ /ddraw/i) {
    push (@libs, 'gdi32', 'ddraw');
    @video = include_targets('video/ddraw/targets.pl');
  } elsif ($video_backend =~ /none/i) {
    @video = include_targets('video/none/targets.pl');
  }
}
push(@video, include_targets('video/common/targets.pl'));
## Done building the video backend ##

## Build the netplay backend ##
if (defined($netplay_backend)) {
  if ($netplay_backend =~ /none/i) {
    @netplay = include_targets('netplay/none/targets.pl');
  } elsif ($netplay_backend =~ /sdl_net/i) {
    @netplay = include_targets('netplay/sdl_net/targets.pl');
    push (@libs, "SDL_net");
  }
}
push(@netplay, include_targets('netplay/common/targets.pl'));
## Done building the netplay backend ##

## Build the input backend ##
if (defined($input_backend)) {
  if ($input_backend =~ /sdl/i) {
    if ($platform =~ /^linux/) {
      #push (@libs, "openal");
    } elsif ($platform =~ /^win32/) {
      #push (@libs, "openal32");
    }
    @input = include_targets('input/sdl/targets.pl');
  } elsif ($input_backend =~ /dinput/i) {
    push (@libs, 'dinput');
    @input = include_targets('input/dinput/targets.pl');
  } elsif ($input_backend =~ /none/i) {
    @input = include_targets('input/none/targets.pl');
  }
}
## Done building the input backend ##

## statically shared objects ##
@common_objs = include_targets('common/targets.pl');
push(@common_objs, include_targets('file/targets.pl'));
@imgfun = include_targets('imgfun/targets.pl');
## end statically shared objects ##

## build game client ##
@client_objs = include_targets('client/targets.pl');
my $client_exe_name = '7kaa';
unless ($disable_wine) {
  $client_exe_name .= '.exe';
}
link_exe ($client_exe_name,
          [@common_objs, @audio, @input, @video, @netplay, @imgfun, @client_objs],
          \@libs,
          \@lib_dirs);
## end build game client ##

## build game server ##
if ($build_server) {
  my $server_exe_name = '7kaa-server';
  unless ($disable_wine) {
    $server_exe_name .= '.exe';
  }
  @server_objs = include_targets('server/targets.pl');
  link_exe ($server_exe_name,
            [@common_objs, @audio, @input, @video, @netplay, @imgfun, @server_objs],
            \@libs,
            \@lib_dirs);
}
## end build game server ##
