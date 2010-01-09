#!/usr/bin/perl

require "opts.pl";

our $msg;

clean_directory();
defined($msg) and print $msg;

1;

sub recurse_dirs {
  foreach my $i (@_) {
    unless (chdir $i) {
      $msg = "clean.pl: directory specified '$i' does not exist.\n";
      return 0; 
    }
    print "Entering '$i'\n";
    clean_directory() or return 0;
    print "Leaving '$i'\n";
    unless (chdir '..') {
      $msg = "clean.pl: parent directory disappeared.\n";
      return 0;
    }
  }
  return 1;
}

sub delete_file {
  if (-f $_[0]) {
    print "Deleting '$_[0]'.\n";
    unless (unlink $_[0]) {
      $msg = "clean.pl: could not delete '$_[0]'.\n";
      return 0;
    }
  }
  return 1;
}

sub clean_directory {
  local @dirs;
  local @obj_files;
  local $exe;

  unless (-f 'targets.pl') {
    $msg = "clean.pl: no targets file. Stopping.\n";
    return 0;
  }

  do 'targets.pl';

  # recurse build directories
  recurse_dirs(@dirs) or return 0;

  # remove object files
  foreach my $i (@obj_files) {
    delete_file($i) or return 0;
  }

  # remove exe files
  if (defined($exe)) {
    delete_file($exe) or return 0;
    delete_file("$exe.so") or return 0;
  }

  return 1;
}
