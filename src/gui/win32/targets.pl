my @defines;

## compiler flags ##
@defines = qw( AMPLUS );
if (defined($debug) && $debug) {
  push (@defines, "DEBUG");
}
if (defined($no_asm) && $no_asm) {
  push (@defines, "NO_ASM");
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
OAUDIO.cpp
OMOUSE.cpp
OVGA.cpp
OVGA2.cpp
OVGABUF.cpp
OVGABUF2.cpp
OVGALOCK.cpp
);

build_targets(\@targets, \@includes, \@defines);
