my @defines;
my @includes;
my @cc_opts;

## compiler flags ##
@defines = qw( AMPLUS );
if (defined($debug) && $debug) {
  push (@defines, "DEBUG");
}
if (defined($no_asm) && $no_asm) {
  push (@defines, "NO_ASM");
}
if ($disable_wine) {
  push (@defines, "NO_WINDOWS");
}
if (defined($audio_backend)) {
  if ($audio_backend =~ /OpenAL/i) {
    push (@defines, 'USE_OPENAL');
  } elsif ($audio_backend =~ /dsound/i) {
    push (@defines, 'USE_DSOUND');
  }
}
if (defined($video_backend)) {
  if ($video_backend =~ /sdl/i) {
    if ($platform =~ /^linux/) {
      my $flags;
      $flags = `sdl-config --cflags`;
      chomp $flags;
      push (@cc_opts, $flags);
    } elsif ($platform =~ /^win32/) {
      # sdl-config is a bash script...which technically works on windows
      # but right now I want to look for better options and hardcode this
      push (@cc_opts, '-D_GNU_SOURCE=1 -Dmain=SDL_main');
    }
    push (@defines, 'USE_SDL');
  } elsif ($video_backend =~ /ddraw/i) {
    push (@defines, 'USE_DDRAW');
  } elsif ($video_backend =~ /none/i) {
    push (@defines, 'USE_NOVIDEO');
  }
}
if (defined($netplay_backend)) {
  if ($netplay_backend =~ /none/i) {
    push (@defines, 'USE_NONETPLAY');
  } elsif ($netplay_backend =~ /sdl_net/i) {
    push (@defines, 'USE_SDLNET');
  }
}
if (defined($input_backend)) {
  if ($input_backend =~ /sdl/i) {
    push (@defines, 'USE_SDL');
  } elsif ($input_backend =~ /dinput/i) {
    push (@defines, 'USE_DINPUT');
  } elsif ($input_backend =~ /none/i) {
    push (@defines, 'USE_NOINPUT');
  }
}
## end compiler flags ##

## include paths ##
@includes = qw( ../../../include );

if (!$disable_wine && defined($wine_prefix)) {
  push (@includes, "$wine_prefix/include/wine/windows",
                   "$wine_prefix/include/wine/msvcrt");
}

if (defined($dxsdk_path)) {
  push (@includes, "$dxsdk_path/include");
}
## include paths ##

@targets = qw(
OERRCTRL.cpp
OREMOTE.cpp
OREMOTE2.cpp
OREMOTEM.cpp
OREMOTEQ.cpp
);

build_targets(\@targets, \@includes, \@defines, \@cc_opts);
