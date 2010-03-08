### Compiler targets ###
@c_files = qw(
OMEM ORESDB
);

if (defined($debug) && $debug) {
  push (@c_files, "dbglog");
}

@defines = qw( AMPLUS );
if (defined($debug) && $debug) {
  push (@defines, "DEBUG");
}

@includes = qw( ../../include );

if (defined($wine_prefix)) {
  push (@includes, "$wine_prefix/include/wine/windows",
                   "$wine_prefix/include/wine/msvcrt");
}

compile(\@c_files, \@includes, \@defines);
