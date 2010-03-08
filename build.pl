#!/usr/bin/perl

use Cwd;
use File::Spec;
use File::stat;

our $top_dir = (File::Spec->splitpath(File::Spec->rel2abs($0)))[1];

our $opts_file = "${top_dir}opts.pl";
is_file_newer("${top_dir}configure.pl", $opts_file) and die "Build options out of date. Please run configure.pl first.\n";

require $opts_file;

include_targets('targets.pl');

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

sub assemble {
  foreach my $i (@_) {
    if (needs_building("$i.asm","$i.o")) {
      my $cmd = "jwasm $jwasm_args -zt1 -Fo $i.o $i.asm";
      print "$cmd\n";
      if (system $cmd) {
        print "build.pl: could not assemble '$i.asm'.\n";
        exit 1;
      }
    }
  }

  return 1;
}

sub compile {
  my ($c_files, $includes, $defines) = @_;
  foreach my $i (@$c_files) {
    if (needs_building("$i.cpp","$i.o")) {
      # OWORLD currently miscompiles at -O2 on gcc 4.3.3.
      # A hack follows--should be handled better or investigated and fixed.
      my @cc_opts;
      push (@cc_opts, $i eq "OWORLD" ? "-O1" : "-O2");
      defined($debug) and $debug and push (@cc_opts, "-g");

      my $cmd = "g++ -c " .
                join(' ', @cc_opts ) . ' ' .
                join(' ', map { "-D$_" } @$defines ) . ' ' .
                join(' ', map { "-I$_" } @$includes ) . ' ' .
                "$i.cpp";
      print "$cmd\n";
      if (system $cmd) {
        print "build.pl: could not compile '$i.cpp'.\n";
        exit 1;
      }
    }
  }
  return 1;
}

sub compile_resources {
  foreach my $i (@_) {
    if (needs_building("$i.rc","$i.o")) {
      my $compiler = $platform =~ /^linux/ ? 'wrc' : 'windres';
      my $cmd = "$compiler -i $i.rc -o $i.o";
      print "$cmd\n";
      if (system $cmd) {
        print "build.pl: couldn't compile resource '$i'\n";
        exit 1;
      }
    }
  }
  return 1;
}

sub link_exe {
  my ($exe, $obj_files, $libs, $lib_dirs) = @_;
  defined($exe) or return 1; # No exe targets here

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
sub include_targets {
  my $orig_dir = cwd;
  foreach my $i (@_) {
    unless (-f $i) {
      print "build.pl: target script '$i' does not exist.\n";
      exit;
    }

    # change directory
    my ($dir, $inc) = (File::Spec->splitpath(File::Spec->rel2abs($i)))[1,2];
    unless (-d $dir && chdir $dir) {
      print "build.pl: unable to enter directory '$dir'.\n";
      exit 1;
    }
    print "Entering '$dir'.\n";

    # run script
    do $inc;

    # go back to the original directory
    print "Leaving '$dir'.\n";
    unless (chdir $orig_dir) {
      print "build.pl: original directory disappeared.\n";
      exit 1;
    }
  }

  return 1;
}

sub build_targets {
  local @dirs;
  local @asm_files;
  local @c_files;
  local @rc_files;
  local @obj_files;
  local @defines;
  local @includes;
  local @libs;
  local @lib_dirs;
  local $exe;

  unless (-f 'targets.pl') {
    print "build.pl: no targets file. Stopping.\n";
    exit 1;
  }

  do 'targets.pl';

  assemble(@asm_files) or return 0;
  compile(\@c_files, \@includes, \@defines) or return 0;
  compile_resources(@rc_files) or return 0;
  link_exe($exe, \@obj_files, \@libs, \@lib_dirs) or return 0;

  return 1;
}
