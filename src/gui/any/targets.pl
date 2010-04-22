my @defines;

## compiler flags ##
@defines = qw( AMPLUS );
if (defined($debug) && $debug) {
  push (@defines, "DEBUG");
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
openal_audio.cpp
wav_stream.cpp
);

build_targets(\@targets, \@includes, \@defines);
