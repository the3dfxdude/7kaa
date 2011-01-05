my @defines;
my @cc_opts;

## compiler flags ##
@defines = qw( AMPLUS USE_NOINPUT );
if (defined($debug) && $debug) {
  push (@defines, "DEBUG");
}
if (defined($no_asm) && $no_asm) {
  push (@defines, "NO_ASM");
}
if ($disable_wine) {
  push (@defines, "NO_WINDOWS");
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
## end compiler flags ##

## include paths ##
my @includes = qw( ../../../include );

if (!$disable_wine && defined($wine_prefix)) {
  push (@includes, "$wine_prefix/include/wine/windows",
                   "$wine_prefix/include/wine/msvcrt");
}
## end include paths ##

my @targets = qw(
OMOUSE.cpp
);

build_targets(\@targets, \@includes, \@defines, \@cc_opts);
