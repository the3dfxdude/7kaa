my @defines;

## compiler flags ##
@defines = qw( AMPLUS );
if (defined($debug) && $debug) {
  push (@defines, "DEBUG");
}
if (defined($no_asm) && $no_asm) {
  push (@defines, "NO_ASM");
}
if (defined($audio_backend)) {
  if ($audio_backend =~ /OpenAL/i) {
    push (@defines, 'USE_OPENAL');
  } elsif ($audio_backend =~ /dsound/i) {
    push (@defines, 'USE_DSOUND');
  }
}
## end compiler flags ##

## include paths ##
my @includes = qw( ../../../include );

if (defined($wine_prefix)) {
  push (@includes, "$wine_prefix/include/wine/windows",
                   "$wine_prefix/include/wine/msvcrt");
}
## end include paths ##

my @targets = qw(
OMOUSE.cpp
OVGA.cpp
OVGA2.cpp
OVGABUF.cpp
OVGABUF2.cpp
OVGALOCK.cpp
syswin.cpp
);

my @objs = build_targets(\@targets, \@includes, \@defines);

## this is will be split out
if (defined($audio_backend) && $audio_backend =~ /dsound/i) {
  my @dsound_defines = qw( AMPLUS USE_DSOUND );
  if (defined($debug) && $debug) {
    push (@dsound_defines, "DEBUG");
  }

  push (@objs, build_targets(['win32_audio.cpp'], \@includes, \@dsound_defines));
}

@objs;
