my @defines;

## compiler flags ##
@defines = qw( AMPLUS USE_SDLNET );
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
## end compiler flags ##

## include paths ##
my @includes = qw( ../../../include );
## end include paths ##

my @targets = qw(
netplay_sdlnet.cpp
);

build_targets(\@targets, \@includes, \@defines);
