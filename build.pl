#!/usr/bin/perl

use Cwd qw(abs_path cwd);
use File::Spec;
use File::stat;

our $top_dir = (File::Spec->splitpath(File::Spec->rel2abs($0)))[1];

our $opts_file = "${top_dir}opts.pl";
is_file_newer("${top_dir}configure.pl", $opts_file) and die "Build options out of date. Please run configure.pl first.\n";

require $opts_file;

my $parallel = (-x "${top_dir}parallel") ? "${top_dir}parallel" : which('parallel');

my $targets_script = defined($ARGV[0]) ? $ARGV[0] : 'targets.pl';
unless (-f $targets_script) {
  print "build.pl: target script '$targets_script' does not exist.\n";
  exit 1;
}
do $targets_script;

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

# get_mtime($file)
sub get_mtime {
  my $file = stat($_[0]);
  return $file->mtime;
}

# is_file_newer($file, $than, \%mtime_cache)
# Checks if $file is newer than $than.
sub is_file_newer {
  (-f $_[1]) or return 1;
  defined($_[2]->{$_[0]}) or $_[2]->{$_[0]} = get_mtime($_[0]);
  defined($_[2]->{$_[1]}) or $_[2]->{$_[1]} = get_mtime($_[1]);
  return $_[2]->{$_[0]} >= $_[2]->{$_[1]};
}

# check_includes($file_to_check, $original_file, $obj, \@inclues, \%mtime_cache)
sub check_includes {
  my $flag = 0;
  my $latest_mtime = 0;
  open (my $file, $_[0]) or return 0;
  while (1) {
    my $line = <$file>;
    defined($line) or last;
    my ($include) = $line =~ /#include\s+<([^>]+)>/;
    if (defined($include)) {
      
      foreach my $i (@{$_[3]}) {
        my $attempt = "$i/$include";
        if (-f $attempt) {
          # recursion is disabled for now
          #unless (defined($_[4]->{$i})) {
          #  my ($ret, $mtime) = check_includes($attempt, $_[1], $_[2], $_[3], $_[4]);
          #  $_[4]->{$attempt} = $mtime;
          #  $ret and $flag = 1;
          #}
          if (is_file_newer($attempt, $_[1], $_[4]) &&
              is_file_newer($attempt, $_[2], $_[4])) {
            $flag = 1;
          }
          $_[4]->{$attempt} > $latest_mtime and $latest_mtime = $_[4]->{$attempt};
          last;
        }
      }

    }
  }
  close ($file);
  return ($flag, $latest_mtime);
}

# needs_building($file, $than, \@includes, \%mtime_cache)
# Checks if targets or build options changed in addition to checking
# whether the object file needs an update.
# C++ files check the immediate include files if they are newer too.
sub needs_building {
  $_[0] =~ /.cpp$/ and (check_includes($_[0], $_[0], $_[1], $_[2], $_[3]))[0] and return 1;
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
  defined($ENV{CFLAGS}) and push (@cc_opts, $ENV{CFLAGS});
  defined($ENV{CXXFLAGS}) and push (@cc_opts, $ENV{CXXFLAGS});
  defined($debug) and $debug and push (@cc_opts, "-g");
  defined($enable_multilib) and $enable_multilib and push (@cc_opts, "-m32");
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
    my $linker = 'g++';
    my @linker_opts;

    if ($platform =~ /^linux/ && !$disable_wine) {
      $linker = 'wineg++';
    }

    defined($ENV{LDFLAGS}) and push(@linker_opts, $ENV{LDFLAGS});
    $debug and push(@linker_opts, '-g');
    defined($enable_multilib) and $enable_multilib and push (@cc_opts, "-m32");

    # windows based compiler options
    unless ($disable_wine) {
      push(@linker_opts, '-mno-cygwin');
      $debug or push(@linker_opts, '-mwindows');
    }
                           
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
  my %mtime_cache;
  my @built_objects;

  my %jobs;

  # Read in input files, catagorize into jobs by extension
  foreach my $i (@{$_[0]}) {
    my ($name, $extension) = break_extension($i);

    unless (defined($extension)) {
      print "build.pl: no extension to figure out the file type of '$i'\n";
      exit 1;
    }

    my $obj = "$name.o";

    if (needs_building($i, $obj, $_[1], \%mtime_cache)) {
      push (@{$jobs{$extension}}, $name);
    }

    push (@built_objects, $obj);
  }


  # execute the jobs
  foreach my $i (keys %jobs) {

    # Using the GNU parallel build system
    if (defined($parallel)) {
      my $cmd;

      # get the command to build this type of file
      if ($i eq 'cpp') {
        my $cxx_cmd = get_cxx_cmd($_[1], $_[2]);
        $cmd = "$cxx_cmd {}.cpp -o {}.o";
      } elsif ($i eq 'asm') {
        my $asm_cmd = get_asm_cmd();
        $cmd = "$asm_cmd -Fo {}.o {}.asm";
      } elsif ($i eq 'rc') {
        my $wrc_cmd = get_wrc_cmd();
        $cmd = "$wrc_cmd -i {}.rc -o {}.o";
      } else {
        print "build.pl: cannot identify how to build file type '$extension'\n";
        exit 1;
      }

      my $manager;
      if (!open($manager, "|-", "$parallel -u -H 1 --verbose '$cmd'")) {
        print "build.pl: couldn't start the parallel build mananger.";
        exit 1;
      }
      foreach my $i (@{$jobs{$i}}) {
        print $manager "$i\n";
      }
      if (!close($manager)) {
        print "build.pl: the build job failed. Stopping.\n";
        exit 1;
      }

    # systems that cannot use GNU parallel
    } else {
      foreach my $j (@{$jobs{$i}}) {
        my $cmd;
        # get the command to build this type of file
        if ($i eq 'cpp') {
          my $cxx_cmd = get_cxx_cmd($_[1], $_[2]);
          $cmd = "$cxx_cmd $j.cpp -o $j.o";
        } elsif ($i eq 'asm') {
          my $asm_cmd = get_asm_cmd();
          $cmd = "$asm_cmd -Fo $j.o $j.asm";
        } elsif ($i eq 'rc') {
          my $wrc_cmd = get_wrc_cmd();
          $cmd = "$wrc_cmd -i $j.rc -o $j.o";
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

    }

  }

  return @built_objects;
}
