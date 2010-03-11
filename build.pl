#!/usr/bin/perl

use Cwd;
use File::Spec;
use File::stat;

our $top_dir = (File::Spec->splitpath(File::Spec->rel2abs($0)))[1];

our $opts_file = "${top_dir}opts.pl";
is_file_newer("${top_dir}configure.pl", $opts_file) and die "Build options out of date. Please run configure.pl first.\n";

require $opts_file;

my $targets_script = defined($ARGV[0]) ? $ARGV[0] : 'targets.pl';
unless (-f $targets_script) {
  print "build.pl: target script '$targets_script' does not exist.\n";
  exit 1;
}
do $targets_script;

1;

# is_file_newer($file, $than)
# Checks if $file is newer than $than.
sub is_file_newer {
  (-f $_[1]) or return 1;
  my $file = stat($_[0]);
  my $than = stat($_[1]);
  return $file->mtime >= $than->mtime;
}

# needs_building($file, $than)
# Checks if targets or build options changed in addition to checking
# whether the object file needs an update.
sub needs_building {
  return is_file_newer($_[0], $_[1]) ||
         is_file_newer('targets.pl', $_[1]) ||
         is_file_newer($opts_file, $_[1]);
}

sub get_asm_cmd {
  return "jwasm $jwasm_args -zt1";
}

# get_cxx_cmd(\@includes, \@defines)
sub get_cxx_cmd {
  my @cc_opts = ('g++', '-c');
  # OWORLD currently miscompiles at -O2 on gcc 4.3.3.
  #push (@cc_opts, $no_asm ? "-O2" : "-O1");
  defined($debug) and $debug and push (@cc_opts, "-g");
  push (@cc_opts, map { "-D$_" } @{$_[1]});
  push (@cc_opts, map { "-I$_" } @{$_[0]});
  return "@cc_opts";
}

sub get_wrc_cmd {
  return $platform =~ /^linux/ ? 'wrc' : 'windres';
}

sub link_exe {
  my ($exe, $obj_files, $libs, $lib_dirs) = @_;
  defined($exe) or return 1; # No exe targets here

print "@$obj_files\n";
  my $flag = 0;
  foreach my $i (@$obj_files) {
    unless (-f $i) {
      print "build.pl: missing file '$i' for linking.\n";
      exit 1;
    }

    # check modification times to see if we have to relink
    if (-f $exe) {
      is_file_newer($i, $exe) and $flag = 1;
    } else {
      $flag = 1;
    }
  }

  if ($flag) {
    my $linker = $platform =~ /^linux/ ? 'wineg++' : 'g++';

    my @linker_opts = ("-g -mno-cygwin");
    $debug or push(@linker_opts, "-mwindows");
                           
    my $cmd = "$linker " .
              join(' ', @linker_opts) . ' ' .
              "@$obj_files " .
              join(' ', map { "-l$_" } @$libs) . ' ' .
              join(' ', map { "-L$_" } @$lib_dirs) . ' ' .
              "-o $exe";
    print $cmd,"\n";
    if (system $cmd) {
      print "build.pl: couldn't create executable '$exe'\n";
      exit 1;
    }
  }

  return 1;
}

# include_targets(@target_files)
#
# Usage: Use in your target files to include build stages found in
# other directories.  This will change directory to where your
# target file is found in and execute the target script.
#
# Note that the new target file executed will have its own clean
# scope.  This forces you to redefine what is necessary to run
# the building.  It makes sense if you are logically grouping
# various targets by their purpose.
#
# Returns paths to the built targets.
#
sub include_targets {
  my @built_targets;
  my $orig_dir = cwd;

  foreach my $i (@_) {
    unless (-f $i) {
      print "build.pl: target script '$i' does not exist.\n";
      exit 1;
    }

    # change directory
    my ($dir, $inc) = (File::Spec->splitpath($i))[1,2];
    ($dir) = $dir =~ /(.+?)\/?$/;
    unless (-d $dir && chdir $dir) {
      print "build.pl: unable to enter directory '$dir'.\n";
      exit 1;
    }
    print "Entering '$dir'.\n";

    # run script
    push (@built_targets, map { "$dir/$_" } do $inc);

    # go back to the original directory
    print "Leaving '$dir'.\n";
    unless (chdir $orig_dir) {
      print "build.pl: original directory disappeared.\n";
      exit 1;
    }
  }

  return @built_targets;
}

sub break_extension {
  my @parts = split('\.', $_[0]);
  my $extension = pop(@parts);
  my $name = join('.', @parts);
  return ($name, $extension);
}

# build_targets(\@files_to_build, \@includes, \@defines)
#
# Usage: Called from target script.  An array of files
# that are to be built is passed.
# i.e. ('AM.cpp','7k.ico', 'IB.asm')
# or qw(AM.cpp 7k.ico IB.asm)
#
# Pass the include paths and defines for the C++ compiler
# as needed for the source files to be built.
#
sub build_targets {
  my $cxx_cmd;
  my $asm_cmd;
  my $wrc_cmd;
  my @built_objects;

  foreach my $i (@{$_[0]}) {
    my ($name, $extension) = break_extension($i);

    unless (defined($extension)) {
      print "build.pl: no extension to figure out the file type of '$i'\n";
      exit 1;
    }

    my $obj = "$name.o";

    if (needs_building($i, $obj)) {
      my $cmd;

      # get the command to build this type of file
      if ($extension eq 'cpp') {
        defined($cxx_cmd) or $cxx_cmd = get_cxx_cmd($_[1], $_[2]);
        $cmd = "$cxx_cmd $i -o $obj";
      } elsif ($extension eq 'asm') {
        defined($asm_cmd) or $asm_cmd = get_asm_cmd();
        $cmd = "$asm_cmd -Fo $name.o $i";
      } elsif ($extension eq 'rc') {
        defined($wrc_cmd) or $wrc_cmd = get_wrc_cmd();
        $cmd = "$wrc_cmd -i $i -o $name.o";
      } else {
        print "build.pl: cannot identify how to build file type '$extension'\n";
        exit 1;
      }

      print "$cmd\n";
      if (system $cmd) {
        print "build.pl: couldn't build '$i'. Stopping.\n";
        exit 1;
      }

    }

    push (@built_objects, $obj);
  }

  return @built_objects;
}
