#!/usr/bin/perl -w

use File::Spec;

my @wine_ver_req = (1, 1, 34);
my @gcc_ver_req = (3, 0, 0);
my @jwasm_ver_req = (2, '00', 0);

# Set-up default config
my %cfg = (
  enable_debug => 0,
  no_asm => 0,
  build_server => 1,
  audio_backend => "OpenAL"
);

# parse command line args
foreach my $i (@ARGV) {
  if ($i =~ /^--with-dxsdk=/) {
    ($cfg{dxsdk_path}) = $i =~ /=(.*)/;
  } elsif ($i =~ /^--enable-debug$/) {
    $cfg{debug} = 1;
  } elsif ($i =~ /^--disable-asm$/) {
    $cfg{no_asm} = 1;
  } elsif ($i =~ /^--disable-server$/) {
    $cfg{build_server} = 0;
  } elsif ($i =~ /^--force-wine$/) {
    @wine_ver_req = (0, 0, 0);
  } elsif ($i =~ /^--with-audio-backend=(.*)$/) {
    $cfg{audio_backend} = $1;
  }
}

# platform
$cfg{platform} = detect_platform();

# assembler setup
unless (check_jwasm_version()) {
  print "JWasm " . join('.', @jwasm_ver_req) . " is required.\n";
  exit 1;
}
$cfg{jwasm_args} = "-q " . get_jwasm_bin_format($cfg{platform});

# compiler setup
unless (check_gcc_version()) {
  print "GCC " . join('.', @gcc_ver_req) . " is required.\n";
  exit 1;
}

if ($cfg{platform} =~ /^linux/) {

  # search for wine
  $cfg{wine_prefix} = detect_wine_prefix();

  unless (defined($cfg{wine_prefix})) {
    print "Wine-" . join('.', @wine_ver_req) . " or later is required.\n";
    exit 1;
  }

} elsif ($cfg{platform} =~ /^win32$/) {

  # search for the DXSDK
  unless (defined($cfg{dxsdk_path})) {
    print "Please specify the DXSDK path with --with-dxsdk=C:/yoursdk\n";
    exit 1;
  }
}

# The following sets flags used during compiling.
# The flags that are known to work:
# AMPLUS
# DISABLE_MULTI_PLAYER
# ENABLE_INTRO_VIDEO
# FRENCH
# GERMAN
# SPANISH
#our @defines = qw(
#AMPLUS
#DISABLE_MULTI_PLAYER
#);

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

# version_check(\@version, \@requirement)
sub version_check {
  my ($ver, $req) = @_;

  # check major number
  $ver->[0] >= $req->[0] or return 0;

  # if major equals
  if ($ver->[0] == $req->[0]) {
    # check medium number
    $ver->[1] >= $req->[1] or return 0;

    # if medium number equals
    if ($ver->[1] == $req->[1]) {
      # check minor number
      $ver->[2] >= $req->[2] or return 0;
    }
  }

  return 1;
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
  my @ver = $wine_version =~ /^wine-(\d+)\.(\d+)\.(\d+)/;
  unless (@ver == 3) {
    print "not found\n";
    return undef;
  }
  print join('.', @ver);
  if (version_check(\@ver, \@wine_ver_req)) {
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

sub check_gcc_version {
  print "Detecting gcc version: ";
  my $gcc_version = `gcc --version`;
  my @ver = $gcc_version =~ /^gcc \(.*\) (\d+)\.(\d+)\.(\d+)/;
  unless (@ver == 3) {
    print "not found\n";
    return undef;
  }
  print join('.', @ver);
  if (version_check(\@ver, \@gcc_ver_req)) {
    print " ok\n";
    return 1;
  }

  print " failed\n";
  return 0;
}

sub check_jwasm_version {
  print "Detecting JWasm version: ";
  my $jwasm_version = `jwasm -?`;
  my @ver = $jwasm_version =~ /^JWasm v(\d+)\.(\d+)/;
  $ver[2] = 0;
  unless (@ver == 3) {
    print "not found\n";
    return undef;
  }
  print join('.', @ver);
  if (version_check(\@ver, \@jwasm_ver_req)) {
    print " ok\n";
    return 1;
  }

  print " failed\n";
  return 0;
}

sub write_config {
  open (my $file, ">opts.pl") or die "Failed to write opts.pl";

  foreach my $i (keys %{$_[0]}) {
    if ($_[0]->{$i} =~ /^\d+$/) {
      print $file "\$$i = $_[0]->{$i};\n";
    } else {
      print $file "\$$i = \"$_[0]->{$i}\";\n";
    }
  }
  print $file "1;\n";

  close ($file);
}
