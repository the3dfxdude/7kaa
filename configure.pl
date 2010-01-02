#!/usr/bin/perl -w

my @wine_ver_req = (1, 1, 34);

my %cfg;

$cfg{platform} = detect_platform();
$cfg{wine_prefix} = detect_wine_prefix();

my @includes = (
"include",
"$cfg{wine_prefix}/include/wine/windows",
"$cfg{wine_prefix}/include/wine/msvcrt",
);
@includes = map { "-I$_" } @includes;
$cfg{includes} = "@includes";

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

write_config(\%cfg);


print "\nReady to run build.pl\n\n";

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
  # Decide where the wine prefix is
  print "Searching for wine: ";
  my $wine_executable = `which wine`;
  chomp $wine_executable;
  if ($wine_executable =~ /^bash/) {
    print "not found -- need wine-" . join('.', @wine_ver_req) . " or later\n";
    die;
  }
  my ($wine_prefix) = $wine_executable =~ /(.*)\/bin/;
  unless (defined($wine_prefix)) {
    print "malformed path\n";
    die;
  }

  # Check the wine version
  print "$wine_prefix\nDetecting wine version: ";
  my $wine_version = `$wine_prefix/bin/wine --version`;
  my @ver = $wine_version =~ /wine-(\d+)\.(\d+)\.(\d+)/;
  unless (@ver == 3) {
    print "unable to run wine: $wine_version\n";
    die;
  }
  print join('.', @ver);
  my $pass = 1;
  $ver[0] >= $wine_ver_req[0] or $pass = 0;
  ($pass and $ver[0] == $wine_ver_req[0] and $ver[1] >= $wine_ver_req[1]) or $pass = 0;
  ($pass and $ver[0] == $wine_ver_req[0] and $ver[1] == $wine_ver_req[1] and $ver[2] >= $wine_ver_req[2]) or $pass = 0;
  if ($pass) {
    print " ok\n";
  } else {
    print " failed -- need wine-" . join('.', @wine_ver_req) . " or later\n";
    die;
  }

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
