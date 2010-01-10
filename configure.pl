#!/usr/bin/perl -w

use File::Spec;

my @wine_ver_req = (1, 1, 34);

my %cfg;

# platform
$cfg{platform} = detect_platform();

# assembler
$cfg{jwasm_args} = "-q " . get_jwasm_bin_format($cfg{platform});

# compiler options
if ($cfg{platform} =~ /^linux/) {

  # search for wine
  $cfg{wine_prefix} = detect_wine_prefix();

  unless (defined($cfg{wine_prefix})) {
    print "Wine-" . join('.', @wine_ver_req) . " or later is required.\n";
    exit 1;
  }

  my @includes = (
  "include",
  "$cfg{wine_prefix}/include/wine/windows",
  "$cfg{wine_prefix}/include/wine/msvcrt",
  );
  @includes = map { "-I$_" } @includes;
  $cfg{includes} = "@includes";
}

# The following sets flags used during compiling.
# The flags that are known to work:
# AMPLUS
# DISABLE_MULTI_PLAYER
# ENABLE_INTRO_VIDEO
# FRENCH
# GERMAN
# SPANISH
our @defines = qw(
AMPLUS
DISABLE_MULTI_PLAYER
);
@defines = map { "-D$_" } @defines;
$cfg{defines} = "@defines";

# write the build options
write_config(\%cfg);


print "\nReady to run build.pl\n\n";

1;

# which($exe_name)
# Perl version to avoid using the system command that does not always exist.
sub which {
  my @path = File::Spec->path();

  foreach my $i (@path) {
    my $loc = "$i/$_[0]";
    (-x $loc) and return $loc;
  }

  return undef;
}

sub get_jwasm_bin_format {
  $_[0] eq 'linux32' and return '-elf';
  $_[0] eq 'win32' and return '-coff';
  die "Don't know bin format for platform '$_[0]' to use with jwasm.\n";
}

sub detect_platform {
  # Detect the platform
  print "Platform: ";
  # if (command line override) {
  if ($^O eq 'linux') {
    if (`uname -m` eq 'x86_64') {
      # probably won't build 64-bit binary, but it needs to be detected and
      # handled
      print "linux64\n";
      return "linux64";
    } else {
      print "linux32\n";
      return "linux32";
    }
  } elsif ($^O eq 'MSWin32') {
    print "win32\n";
    return "win32";
  } else {
    print '$^O (unsupported)\n';
    die;
  }
}

sub detect_wine_prefix {
  # Check the wine version
  print "Detecting wine version: ";
  my $wine_version = `wine --version`;
  my @ver = $wine_version =~ /wine-(\d+)\.(\d+)\.(\d+)/;
  unless (@ver == 3) {
    print "not found\n";
    return undef;
  }
  print join('.', @ver);
  my $pass = 1;
  $ver[0] >= $wine_ver_req[0] or $pass = 0;
  ($pass and $ver[0] == $wine_ver_req[0] and $ver[1] >= $wine_ver_req[1]) or $pass = 0;
  ($pass and $ver[0] == $wine_ver_req[0] and $ver[1] == $wine_ver_req[1] and $ver[2] >= $wine_ver_req[2]) or $pass = 0;
  if ($pass) {
    print " ok\n";
  } else {
    print " failed\n";
    return undef;
  }

  # Decide where the wine prefix is
  print "Searching for wine: ";
  my $wine_executable = which('wine');
  unless (defined($wine_executable)) {
    print "not found\n";
    return undef;
  }
  my ($wine_prefix) = $wine_executable =~ /(.*)\/bin/;
  unless (defined($wine_prefix)) {
    print "malformed path\n";
    return undef;
  }
  print "$wine_prefix\n";

  return $wine_prefix;
}

sub write_config {
  open (my $file, ">opts.pl") or die "Failed to write opts.pl";

  foreach my $i (keys %{$_[0]}) {
    print $file "\$$i = \"$_[0]->{$i}\";\n";
  }
  print $file "1;\n";

  close ($file);
}
