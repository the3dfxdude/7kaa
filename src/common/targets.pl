### Compiler targets ###
if (defined($debug) && $debug) {
  push (@c_files, "dbglog");
}

@defines = qw( AMPLUS );
if (defined($debug) && $debug) {
  push (@defines, "DEBUG");
}

@includes = qw( ../../include );
