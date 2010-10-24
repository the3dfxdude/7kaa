#!/usr/bin/perl

use Cwd;

our $msg;

clean_build_files();
defined($msg) and print $msg;

1;

sub recurse_dirs {
  foreach my $i (@_) {
    my $orig_dir = cwd;

    unless (-d $i && chdir $i) {
      $msg = "clean.pl: could not enter '$i'.\n";
      return 0; 
    }
    print "Entering '$i'\n";

    clean_build_files($i) or return 0;

    print "Leaving '$i'\n";
    unless (chdir $orig_dir) {
      $msg = "clean.pl: original directory disappeared.\n";
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

sub clean_build_files {
  # recurse build directories
  opendir (my $dir, '.') or die "Cannot read current directory.\n";
  my @dirs = grep { -d $_ && not $_ =~ /^\./ } readdir($dir);
  closedir ($dir);
  recurse_dirs(@dirs) or return 0;

  ## Clean up build files ##
  my @files;

  # remove object files
  @files = <*.o>;
  foreach my $i (@files) {
    delete_file($i) or return 0;
  }

  # remove exe files
  @files = <*.exe*>;
  foreach my $i (@files) {
    delete_file($i) or return 0;
  }
  @files = <7kaa*>;
  foreach my $i (@files) {
    if (-f $i && -x $i) {
      delete_file($i) or return 0;
    }
  }

  return 1;
}
