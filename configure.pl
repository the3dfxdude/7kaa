#!/usr/bin/perl -w

use Cwd qw(abs_path);
use File::Spec;

my @wine_ver_req = (1, 1, 34);
my @gcc_ver_req = (3, 0, 0);
my @jwasm_ver_req = (2, '00', 0);

# Set-up default config
my %cfg = (
  debug => 1,
  no_asm => 1,
  build_server => 0,
  audio_backend => "OpenAL",
  video_backend => "sdl",
  netplay_backend => "sdl_net",
  input_backend => "sdl",
  disable_wine => 1,
  enable_multilib => 0
);

# parse command line args
foreach my $i (@ARGV) {
  if ($i =~ /^--with-dxsdk=/) {
    ($cfg{dxsdk_path}) = $i =~ /=(.*)/;
    $cfg{video_backend} = 'ddraw';
    $cfg{input_backend} = 'dinput';
  } elsif ($i =~ /^--enable-debug$/) {
    $cfg{debug} = 1;
  } elsif ($i =~ /^--disable-debug$/) {
    $cfg{debug} = 0;
  } elsif ($i =~ /^--disable-asm$/) {
    $cfg{no_asm} = 1;
  } elsif ($i =~ /^--enable-asm$/) {
    $cfg{no_asm} = 0;
#  } elsif ($i =~ /^--disable-server$/) {
#    $cfg{build_server} = 0;
#  } elsif ($i =~ /^--enable-server$/) {
#    $cfg{build_server} = 1;
  } elsif ($i =~ /^--force-wine$/) {
    @wine_ver_req = (0, 0, 0);
  } elsif ($i =~ /^--enable-wine$/) {
    $cfg{disable_wine} = 0;
    $cfg{video_backend} = 'ddraw';
    $cfg{input_backend} = 'dinput';
  } elsif ($i =~ /^--disable-wine$/) {
    $cfg{disable_wine} = 1;
    $cfg{video_backend} = 'sdl';
    $cfg{input_backend} = 'sdl';
  } elsif ($i =~ /^--with-audio-backend=(.*)$/) {
    $cfg{audio_backend} = $1;
  } elsif ($i =~ /^--enable-multilib$/) {
    $cfg{enable_multilib} = 1;
  } elsif ($i =~ /^--enable-sdlnet$/) {
    $cfg{netplay_backend} = "sdl_net",
  } elsif ($i =~ /^--help$/) {
    print "Call configure.pl with any of the following options:\n";
    print "--disable-debug: Do not compile in extra debugging code\n";
    print "--enable-wine: Use Winelib on x86 linux\n";
    print "--enable-asm: Use old 386 asm code (needs JWasm 2.x)\n";
    print "--enable-multilib: Compile 32-bit binary on 64-bit cpu architecture.\n";
    print "The default settings builds a native game binary for sdl and openal.\n";
    exit 0;
  }
}

# platform
$cfg{platform} = detect_platform();

# assembler setup
unless ($cfg{no_asm} || check_jwasm_version()) {
  print "JWasm " . join('.', @jwasm_ver_req) . " is required.\n";
  exit 1;
}
$cfg{jwasm_args} = "-q " . get_jwasm_bin_format($cfg{platform});

# compiler setup
unless (check_gcc_version('gcc')) {
  print "GCC " . join('.', @gcc_ver_req) . " is required.\n";
  exit 1;
}
unless (check_gcc_version('g++')) {
  print "GCC " . join('.', @gcc_ver_req) . " is required.\n";
  exit 1;
}

if ($cfg{platform} =~ /^linux/) {

  if (!$cfg{disable_wine}) {
    # search for wine
    $cfg{wine_prefix} = detect_wine_prefix();

    unless (defined($cfg{wine_prefix})) {
      print "Wine-" . join('.', @wine_ver_req) . " or later is required.\n";
      exit 1;
    }
  }

  if ($cfg{video_backend} =~ /^sdl$/i || $cfg{input_backend} =~ /^sdl$/i) {
    # search for SDL files
    print "Checking for SDL: ";
    my $sdl_config = which("sdl-config");
    if (!defined($sdl_config)) {
      print "not found\n";
      print "SDL library and development files are required.\n";
      exit 1;
    }
    print "found\n";

    if ($cfg{netplay_backend} =~ /^sdl_net$/i) {
      my $sdl_libs = `$sdl_config --libs`;
      chomp $sdl_libs;
      my $sdl_cflags = `$sdl_config --cflags`;
      chomp $sdl_cflags;

      # test for SDL_net (requires SDL)
      print "Checking for SDL_net: ";

      my $program = <<EOF;
#include <SDL.h>
#include <SDL/SDL_net.h>
int main()
{
SDL_version compile_version;
const SDL_version *linked_version;
SDL_NET_VERSION(&compile_version)
linked_version = SDLNet_Linked_Version();
return 0;
}
EOF

      if (!compile_test($program, "$sdl_libs $sdl_cflags -lSDL_net")) {
        # fall back to none
        $cfg{netplay_backend} = "none";
        print "Disabling network backend!\n";
      }
    }

  }

  if ($cfg{audio_backend} =~ /^openal$/i) {
    # search for OpenAL files
    print "Checking for OpenAL: ";

    my $program = <<EOF;
#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
int main()
{
ALCdevice *al_device = alcOpenDevice(NULL);
return 0;
}
EOF
    if (!compile_test($program, "-lopenal")) {
      # fall back to none
      $cfg{audio_backend} = "none";
      print "Disabling audio backend!\n";
    }
  }

} elsif ($cfg{platform} =~ /^win32$/) {

  # wine and windows are equivalent, and this is required on windows
  $cfg{disable_wine} = 0;

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

  # check the path for the executable
  foreach my $i (@path) {
    my $loc = "$i/$_[0]";
    (-x $loc) and return abs_path($loc);
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
  $_[0] eq 'linux64' and return '-elf';
  $_[0] eq 'win32' and return '-coff';
  die "Don't know bin format for platform '$_[0]' to use with jwasm.\n";
}

sub detect_platform {
  # Detect the platform
  print "Platform: ";
  # if (command line override) {
  if ($^O eq 'linux') {
    my $arch = `uname -m`;
    chomp $arch;
    if ($arch eq 'x86_64') {
      # probably won't build 64-bit binary, but it needs to be detected and
      # handled
      print "linux64\n";
      return "linux64";
    } else {
      print "linux32\n";
      return "linux32";
    }
  } elsif ($^O eq 'MSWin32' || $^O eq 'msys') {
    print "win32\n";
    return "win32";
  } else {
    print "$^O (unsupported)\n";
    exit 1;
  }
}

sub detect_wine_prefix {
  # Check the wine version
  print "Detecting wine version: ";
  my $wine_version = `wine --version`;
  my @ver = split ('-', $wine_version);
  my @dots = split ('\.', $ver[1]);
  defined($dots[2]) or $dots[2] = 0;
  print join('.', @dots);
  if (version_check(\@dots, \@wine_ver_req)) {
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

# check_gcc_version($executable_name)
sub check_gcc_version {
  print "Checking for $_[0]: ";
  my $gcc_version = `$_[0] --version`;
  my @parts = split(/\s+/, $gcc_version);
  my @ver = split(/\./, $parts[2]);
  $ver[2] = 0;
  unless (@ver == 3) {
    print "couldn't read gcc version correctly\n";
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

# compile_test($program, $cc_opts)
sub compile_test {
  open (my $prgfile, ">nnnnnnn.c") or die "Couldn't write file: $!";
  print $prgfile $_[0];
  close ($prgfile);

  my $cc;
  my $cmd = "gcc nnnnnnn.c -o nnnnnnn $_[1] 2>&1 |";
  if (!open ($cc, $cmd)) {
    print "failed (couldn't open pipe: $!)\n";
    return 0;
  }
  my @lines;
  while (1) {
    my $line = <$cc>;
    defined($line) or last;
    push (@lines, $line);
  }
  $ret = close($cc);

  unlink "nnnnnnn";
  unlink "nnnnnnn.c";
  if (!$ret) {
    # error during compilation
    print "not found\n";
    print join("\n", @lines);
    return 0;
  }
  print "found\n";
  return 1;
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
