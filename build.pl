#!/usr/bin/perl

use File::stat;

(-f 'opts.pl') or die "Please run configure.pl first.\n";
require "opts.pl";

our $msg;

build_targets();
defined($msg) and print $msg;

1;

# is_file_newer($file, $than)
# Checks if $file is newer than $than.
sub is_file_newer {
  (-f $_[1]) or return 1;
  my $file = stat($_[0]);
  my $than = stat($_[1]);
  return $file->mtime >= $than->mtime;
}

sub assemble {
  foreach my $i (@_) {
    if (is_file_newer("$i.asm","$i.o")) {
      my $cmd = "jwasm $jwasm_args -zt1 -Fo $i.o $i.asm";
      print "$cmd\n";
      system $cmd and $msg = "build.pl: could not assemble '$i.asm'.\n" and return 0;
    }
  }

  return 1;
}

sub compile {
  my ($c_files, $includes, $defines) = @_;
  foreach my $i (@$c_files) {
    if (is_file_newer("$i.cpp","$i.o")) {
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
      system $cmd and $msg = "build.pl: could not compile '$i.cpp'.\n" and return 0;
    }
  }
  return 1;
}

sub compile_resources {
  foreach my $i (@_) {
    if (is_file_newer("$i.rc","$i.o")) {
      my $compiler = $platform =~ /^linux/ ? 'wrc' : 'windres';
      my $cmd = "$compiler -i $i.rc -o $i.o";
      print "$cmd\n";
      if (system $cmd) {
        $msg = "build.pl: couldn't compile resource '$i'\n" and
        return 0;
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
      $msg = "build.pl: missing file '$i' for linking.\n";
      return 0;
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
    defined($debug) and !$debug and push(@linker_opts, "-mwindows");
                           
    my $cmd = "$linker " .
              join(' ', $linker_opts) . ' ' .
              "@$obj_files " .
              join(' ', map { "-l$_" } @$libs) . ' ' .
              join(' ', map { "-L$_" } @$lib_dirs) . ' ' .
              "-o $exe";
    print $cmd,"\n";
    if (system $cmd) {
      $msg = "build.pl: couldn't create executable '$exe'\n" and
      return 0;
    }
  }

  return 1;
}

sub recurse_dirs {
  foreach my $i (@_) {
    unless (chdir $i) {
      $msg = "build.pl: directory specified '$i' does not exist.\n";
      return 0;
    }
    print "Entering '$i'.\n";
    build_targets() or return 0;
    print "Leaving '$i'.\n";
    unless (chdir '..') {
      $msg = "build.pl: parent directory disappeared.\n";
      return 0;
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
    $msg = "build.pl: no targets file. Stopping.\n";
    return 0;
  }

  do 'targets.pl';

  recurse_dirs(@dirs) or return 0;
  assemble(@asm_files) or return 0;
  compile(\@c_files, \@includes, \@defines) or return 0;
  compile_resources(@rc_files) or return 0;
  link_exe($exe, \@obj_files, \@libs, \@lib_dirs) or return 0;

  return 1;
}
