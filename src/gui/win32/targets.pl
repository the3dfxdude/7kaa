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
  push (@defines, "AUDIO_BACKEND=$audio_backend");
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
win32_audio.cpp
);

build_targets(\@targets, \@includes, \@defines);
