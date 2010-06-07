## compiler flags ##
@defines = qw( AMPLUS );
if (defined($debug) && $debug) {
  push (@defines, "DEBUG");
}
if (defined($no_asm) && $no_asm) {
  push (@defines, "NO_ASM");
}
if (defined($video_backend)) {
  if ($video_backend =~ /sdl/i) {
    push (@defines, 'USE_SDL');
  } elsif ($video_backend =~ /ddraw/i) {
    push (@defines, 'USE_DDRAW');
  } elsif ($video_backend =~ /none/i) {
    push (@defines, 'USE_NOVIDEO');
  }
}
## end compiler flags ##

## include paths ##
@includes = qw( ../../include );

if (defined($wine_prefix)) {
  push (@includes, "$wine_prefix/include/wine/windows",
                   "$wine_prefix/include/wine/msvcrt");
}
## end include paths ##

## compile ##
@targets = qw(
OCONFIG.cpp
OERROR.cpp
OFILE.cpp
OMEM.cpp
ORESDB.cpp
file_input_stream.cpp
file_util.cpp
mem_input_stream.cpp
);
if (defined($debug) && $debug) {
  push (@targets, 'dbglog.cpp');
}
build_targets(\@targets, \@includes, \@defines);
## end compile ##
