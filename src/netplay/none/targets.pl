my @defines;
my @includes;

## compiler flags ##
@defines = qw( AMPLUS USE_NONETPLAY );
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
## end include paths ##

my @targets = qw(
netplay_none.cpp
);

build_targets(\@targets, \@includes, \@defines);
